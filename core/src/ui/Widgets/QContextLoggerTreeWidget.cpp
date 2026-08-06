#include "ui/Widgets/QContextLoggerTreeWidget.h"
#include "LogManager.h"
#include <algorithm>

#ifdef QT_WIDGETS_LIB
#include <QApplication>
#include <QClipboard>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QKeyEvent>
#include <functional>

namespace {
    // Read-only in-place editor: click to enter text-selection mode; a
    // double-click (view or editor) copies the row's message text. Escape
    // (pressed while the editor has focus) invokes the owner's deselect
    // handler.
    class ReadOnlyLineEditDelegate : public QStyledItemDelegate
    {
    public:
        ReadOnlyLineEditDelegate(int messageColumn, std::function<void()> onEscape, QObject* parent)
            : QStyledItemDelegate(parent), m_messageColumn(messageColumn), m_onEscape(std::move(onEscape)) {}

        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            QLineEdit* editor = new QLineEdit(parent);
            editor->setReadOnly(true);
            editor->setFrame(false);
            editor->setFont(option.font);
            editor->setAutoFillBackground(true);
            editor->setContextMenuPolicy(Qt::NoContextMenu);
            editor->setTextMargins(0, 0, 0, 0);
            editor->setContentsMargins(0, 0, 0, 0);
            editor->setAlignment(option.displayAlignment != 0 ? option.displayAlignment : (Qt::AlignLeft | Qt::AlignVCenter));

            QPalette pal = editor->palette();
            const QVariant bg = index.data(Qt::BackgroundRole);
            if (bg.isValid() && bg.canConvert<QBrush>())
            {
                const QColor bgc = bg.value<QBrush>().color();
                if (bgc.isValid())
                {
                    pal.setColor(QPalette::Base, bgc);
                    pal.setColor(QPalette::Window, bgc);
                }
            }
            const QVariant fg = index.data(Qt::ForegroundRole);
            if (fg.isValid() && fg.canConvert<QBrush>())
            {
                const QColor fgc = fg.value<QBrush>().color();
                if (fgc.isValid() && fgc.alpha() > 0)
                    pal.setColor(QPalette::Text, fgc);
            }
            // Keep selection highlight visible even when the editor loses focus
            // (Qt would otherwise switch to the Inactive palette group, which on
            // most themes renders the selection as invisible or nearly so).
            pal.setColor(QPalette::Inactive, QPalette::Highlight,
                pal.color(QPalette::Active, QPalette::Highlight));
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                pal.color(QPalette::Active, QPalette::HighlightedText));
            editor->setPalette(pal);

            editor->setText(index.data(Qt::DisplayRole).toString());
            editor->setProperty("__persistentIndex", QVariant::fromValue(QPersistentModelIndex(index)));
            editor->installEventFilter(const_cast<ReadOnlyLineEditDelegate*>(this));
            return editor;
        }
        void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const override {}
        // Override to preserve the QLineEdit's text selection and cursor across
        // model refreshes. Qt calls setEditorData on the persistent editor whenever
        // the view thinks the underlying data may have changed (e.g. when siblings
        // are inserted); the default implementation calls setText() which resets
        // selection — that's what was clearing the user's selection on new messages.
        void setEditorData(QWidget* editor, const QModelIndex& index) const override
        {
            QLineEdit* le = qobject_cast<QLineEdit*>(editor);
            if (!le)
            {
                QStyledItemDelegate::setEditorData(editor, index);
                return;
            }
            const QString newText = index.data(Qt::DisplayRole).toString();
            if (le->text() == newText)
                return;
            const int selStart = le->selectionStart();
            const int selLen = le->selectedText().length();
            const int cursor = le->cursorPosition();
            const bool wasBlocked = le->blockSignals(true);
            le->setText(newText);
            if (selStart >= 0 && selLen > 0)
                le->setSelection(selStart, selLen);
            else
                le->setCursorPosition(qMin(cursor, newText.length()));
            le->blockSignals(wasBlocked);
        }

        bool eventFilter(QObject* obj, QEvent* ev) override
        {
            if (ev->type() == QEvent::KeyPress)
            {
                QKeyEvent* ke = static_cast<QKeyEvent*>(ev);
                if (ke->key() == Qt::Key_Escape && m_onEscape)
                {
                    // Consume before QStyledItemDelegate's default Escape
                    // handling, which would close the editor widget but leave
                    // the owner's selection bookkeeping (and follow-pause) set.
                    m_onEscape();
                    return true;
                }
            }
            if (ev->type() == QEvent::MouseButtonDblClick)
            {
                QLineEdit* editor = qobject_cast<QLineEdit*>(obj);
                if (editor)
                {
                    QPersistentModelIndex idx = editor->property("__persistentIndex").value<QPersistentModelIndex>();
                    if (idx.isValid())
                    {
                        const QString text = idx.model()->index(idx.row(), m_messageColumn, idx.parent())
                            .data(Qt::DisplayRole).toString();
                        if (!text.isEmpty())
                            QApplication::clipboard()->setText(text);
                        return true;
                    }
                }
            }
            return QStyledItemDelegate::eventFilter(obj, ev);
        }
    private:
        int m_messageColumn;
        std::function<void()> m_onEscape;
    };
}

namespace Log
{
	namespace UIWidgets
	{
		QContextLoggerTreeWidget::QContextLoggerTreeWidget(QTreeWidget* parent)
			: QWidget(parent)
			, m_treeWidget(parent)
		{
			for(int i = 0; i < sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]); ++i)
			{
				m_levelVisibility[i] = true;
			}

			m_dateTimeFilter.active = false;
			m_timeFormat = DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond;
			m_updateTimer.setInterval(250);
			connect(&m_updateTimer, &QTimer::timeout, this, &QContextLoggerTreeWidget::onUpdateTimer);
			m_updateTimer.start();

			m_treeWidget->setColumnCount(3);
			m_treeWidget->setUniformRowHeights(true);
			m_treeWidget->setAnimated(false);
			QStringList headerLables;
			for (int i = 0; i < (int)HeaderPos::__count; ++i)
			{
				headerLables << getHeaderName((HeaderPos)i);
				m_treeWidget->setColumnWidth(i, getHeaderWidth((HeaderPos)i));
			}
			m_treeWidget->setHeaderLabels(headerLables);

			// In-cell text selection: a persistent read-only line editor is opened
			// on whichever cell the user clicks, so drag-select works and survives
			// new messages arriving. Row selection is disabled — the user selects
			// text, not rows.
			m_treeWidget->setItemDelegate(new ReadOnlyLineEditDelegate(
				(int)HeaderPos::message,
				[this]() { clearTextSelection(); },
				m_treeWidget));
			m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);

			// Open the persistent editor only on a genuine cell CLICK.
			// currentItemChanged is NOT a safe trigger: Qt also moves the
			// current item on focus restore (e.g. returning from another tab),
			// and an editor opened by that is invisible to the user while
			// silently blocking stick-to-bottom.
			connect(m_treeWidget, &QTreeWidget::itemPressed, this,
				[this](QTreeWidgetItem* item, int column)
				{
					if (!item)
						return;
					if (!(QGuiApplication::mouseButtons() & Qt::LeftButton))
						return;
					if (m_editorItem == item && m_editorColumn == column)
						return; // already open on this cell
					if (m_editorItem)
						m_treeWidget->closePersistentEditor(m_editorItem, m_editorColumn);
					m_treeWidget->openPersistentEditor(item, column);
					m_editorItem = item;
					m_editorColumn = column;
				});

			connect(m_treeWidget, &QTreeWidget::currentItemChanged, this,
				[this](QTreeWidgetItem* current, QTreeWidgetItem* previous)
				{
					Q_UNUSED(previous);
					// Close the editor of the item we're leaving. The new
					// editor (if any) is opened by the click handler, not here.
					if (m_editorItem && m_editorItem != current)
					{
						m_treeWidget->closePersistentEditor(m_editorItem, m_editorColumn);
						m_editorItem = nullptr;
						m_editorColumn = -1;
					}
					if (current)
					{
						// Look up MessageData for details pane.
						for (const auto& kv : m_msgItems)
						{
							for (const auto& md : kv.second->msgItems)
							{
								if (md.item == current)
								{
									emit selectionChangedMessage(md.msg, true);
									return;
								}
							}
						}
						emit selectionChangedMessage(Log::Message(), false);
					}
					else
					{
						emit selectionChangedMessage(Log::Message(), false);
					}
				});

			// Stick-to-bottom mechanic (same design as QConsoleWidget):
			//  - actionTriggered fires only for user scroll input and arms the
			//    marker; the directly following valueChanged (same call stack)
			//    evaluates the flag with value and maximum sampled together.
			//  - Qt-internal value changes (layout, tab switches) are unarmed
			//    and therefore ignored.
			//  - rangeChanged re-clamps to the new maximum while sticky, which
			//    also covers geometry settling after a tab switch.
			connect(m_treeWidget->verticalScrollBar(), &QAbstractSlider::actionTriggered,
				this, [this](int) { m_userScrollAction = true; });
			connect(m_treeWidget->verticalScrollBar(), &QAbstractSlider::valueChanged,
				this, [this](int value)
				{
					const bool userAction = m_userScrollAction;
					m_userScrollAction = false;
					if (m_programmaticScroll)
						return;
					if (!userAction)
						return;
					if (!m_treeWidget->isVisible())
						return;
					const int max = m_treeWidget->verticalScrollBar()->maximum();
					m_stickToBottom = (max - value <= 1);
				});
			connect(m_treeWidget->verticalScrollBar(), &QScrollBar::rangeChanged,
				this, [this](int, int max)
				{
					if (m_stickToBottom && !m_editorItem)
					{
						m_programmaticScroll = true;
						m_treeWidget->verticalScrollBar()->setValue(max);
						m_programmaticScroll = false;
					}
				});

			// Escape clears the in-cell selection (when the tree itself has
			// focus; the delegate handles Escape while an editor has focus).
			m_treeWidget->installEventFilter(this);
			// Double-click copies the row's message text to the clipboard.
			connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this,
				[](QTreeWidgetItem* item, int /*column*/)
				{
					if (!item)
						return;
					const QString text = item->data((int)HeaderPos::message, Qt::DisplayRole).toString();
					if (!text.isEmpty())
						QApplication::clipboard()->setText(text);
				});

			// Right-click context menu on tree items.
			m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this,
				[this](const QPoint& pos)
				{
					if (!m_contextMenuEnabled)
						return;
					QTreeWidgetItem* item = m_treeWidget->itemAt(pos);
					if (!item)
						return;
					showRowContextMenu(item, m_treeWidget->viewport()->mapToGlobal(pos));
				});
		}

		QContextLoggerTreeWidget::~QContextLoggerTreeWidget()
		{

		}

		const QString& QContextLoggerTreeWidget::getHeaderName(HeaderPos pos) const
		{
			switch (pos)
			{
			case HeaderPos::contextName: { static QString s = "Context"; return s; }
			case HeaderPos::timestamp: { static QString s = "Timestamp"; return s; }
			case HeaderPos::message: { static QString s = "Message"; return s; }
			}
			static QString s;
			return s;
		}
		unsigned int QContextLoggerTreeWidget::getHeaderWidth(HeaderPos pos) const
		{
			switch (pos)
			{
			case HeaderPos::contextName: { return 200; }
			case HeaderPos::timestamp: { return 150; }
			case HeaderPos::message: { return 500; }
			}
			return 0;
		}
		void QContextLoggerTreeWidget::setDateTimeFormat(DateTime::Format format)
		{
			if(m_timeFormat == format)
				return;
			m_timeFormat = format;
			for (auto& context : m_msgItems)
			{
				context.second->updateDateTime();
			}
		}
		DateTime::Format QContextLoggerTreeWidget::getDateTimeFormat() const
		{
			return m_timeFormat;
		}

		void QContextLoggerTreeWidget::addContext(const LogObject::Info &newContext)
		{
			if (m_msgItems.find(newContext.id) != m_msgItems.end())
				return;

			LoggerID parentID = newContext.parentId;

			if (parentID > 0)
			{
				const auto& parentIt = m_msgItems.find(parentID);
				if (parentIt == m_msgItems.end())
				{
					TreeData* treeData = new TreeData(this, newContext);
					m_msgItems[newContext.id] = treeData;
					return;
				}

				TreeData* parentTreeData = parentIt->second;
				m_msgItems[newContext.id] = parentTreeData->createChild(newContext);
				return;
			}
			TreeData *treeData = new TreeData(this, newContext);
			m_msgItems[newContext.id] = treeData;
		}
		void QContextLoggerTreeWidget::onNewMessage(const Message& m)
		{
			const auto &it = m_msgItems.find(m.getLoggerID());
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->onNewMessage(m);
			m_messageCountDirty = true;
		}
		void QContextLoggerTreeWidget::onNewMessages(const std::vector<Message>& messages)
		{
			if (messages.empty())
				return;

			m_treeWidget->setUpdatesEnabled(false);
			for (const Message& message : messages)
				onNewMessage(message);
			m_treeWidget->setUpdatesEnabled(true);
			// Keep the view anchored to the newest message unless the user
			// scrolled up or is holding an in-cell text selection.
			if (m_stickToBottom && !m_editorItem)
				scrollToBottomGuarded();
		}
		void QContextLoggerTreeWidget::clearTextSelection()
		{
			if (m_editorItem)
			{
				m_treeWidget->closePersistentEditor(m_editorItem, m_editorColumn);
				m_editorItem = nullptr;
				m_editorColumn = -1;
			}
			// Clearing the current item also clears the details pane via
			// currentItemChanged. With no editor left, auto-follow resumes on
			// the next reconciliation if the view is still stick-to-bottom.
			m_treeWidget->setCurrentItem(nullptr);
			m_treeWidget->setFocus();
		}

		bool QContextLoggerTreeWidget::eventFilter(QObject* obj, QEvent* ev)
		{
			if (obj == m_treeWidget && ev->type() == QEvent::KeyPress)
			{
				QKeyEvent* ke = static_cast<QKeyEvent*>(ev);
				if (ke->key() == Qt::Key_Escape &&
					(m_editorItem || m_treeWidget->currentItem()))
				{
					clearTextSelection();
					return true;
				}
			}
			return QWidget::eventFilter(obj, ev);
		}

		void QContextLoggerTreeWidget::scrollToBottomGuarded()
		{
			m_programmaticScroll = true;
			m_treeWidget->scrollToBottom();
			m_programmaticScroll = false;
		}

		void QContextLoggerTreeWidget::clearMessages()
		{
			// The editor's item is about to be deleted — drop the bookkeeping
			// so it can't dangle.
			m_editorItem = nullptr;
			m_editorColumn = -1;
			m_treeWidget->setUpdatesEnabled(false);
			for (auto& it : m_msgItems)
			{
				it.second->clearMessages();
			}
			m_treeWidget->setUpdatesEnabled(true);
			m_messageCountDirty = true;
		}


		void QContextLoggerTreeWidget::setDateTimeFilter(const DateTimeFilter& filter)
		{
			m_dateTimeFilter = filter;
			updateDateTimeFilter();
		}
		const DateTimeFilter& QContextLoggerTreeWidget::getDateTimeFilter() const
		{
			return m_dateTimeFilter;
		}
		void QContextLoggerTreeWidget::setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType)
		{
			m_dateTimeFilter.min = min;
			m_dateTimeFilter.max = max;
			m_dateTimeFilter.rangeType = rangeType;
			m_dateTimeFilter.active = true;
			updateDateTimeFilter();
		}
		void QContextLoggerTreeWidget::clearDateTimeFilter()
		{
			m_dateTimeFilter.active = false;
			updateDateTimeFilter();
		}
		const DateTime& QContextLoggerTreeWidget::getDateTimeFilterMin() const
		{
			return m_dateTimeFilter.min;
		}
		const DateTime& QContextLoggerTreeWidget::getDateTimeFilterMax() const
		{
			return m_dateTimeFilter.max;
		}
		DateTime::Range QContextLoggerTreeWidget::getDateTimeFilterRangeType() const
		{
			return m_dateTimeFilter.rangeType;
		}
		bool QContextLoggerTreeWidget::isDateTimeFilterActive() const
		{
			return m_dateTimeFilter.active;
		}
		void QContextLoggerTreeWidget::setTextFilter(const QString& text, bool useRegex)
		{
			QString effective = text;
			m_searchNegate = effective.startsWith('!');
			if (m_searchNegate)
				effective = effective.mid(1);
			m_searchText = effective;
			m_searchUseRegex = useRegex;
			if (useRegex && !effective.isEmpty())
				m_searchRegex = QRegularExpression(effective, QRegularExpression::CaseInsensitiveOption);
			else
				m_searchRegex = QRegularExpression();

			auto matcher = [this](const std::string& text) { return matchesSearchText(text); };
			m_treeWidget->setUpdatesEnabled(false);
			for (auto& it : m_msgItems)
				it.second->applyTextFilter(matcher);
			m_treeWidget->setUpdatesEnabled(true);
			m_messageCountDirty = true;
		}
		bool QContextLoggerTreeWidget::matchesSearchText(const std::string& text) const
		{
			if (m_searchText.isEmpty())
				return true;
			const QString msg = QString::fromStdString(text);
			bool hit;
			if (m_searchUseRegex)
			{
				if (!m_searchRegex.isValid())
					return true;
				hit = m_searchRegex.match(msg).hasMatch();
			}
			else
			{
				hit = msg.contains(m_searchText, Qt::CaseInsensitive);
			}
			return hit != m_searchNegate;
		}
		void QContextLoggerTreeWidget::setParent(LoggerID childID, LoggerID parentID)
		{
			TreeData* child = nullptr;
			TreeData* parent = nullptr;
			
			const auto &childIt = m_msgItems.find(childID);
			if (childIt != m_msgItems.end())
				child = childIt->second;

			const auto &parentIt = m_msgItems.find(parentID);
			if (parentIt != m_msgItems.end())
				parent = parentIt->second;
			
			if (child && parent)
			{
				child->setParent(parent);
				m_messageCountDirty = true;
			}
		}
		void QContextLoggerTreeWidget::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
		{
			for (auto& it : m_msgItems)
			{
				it.second->saveVisibleMessages(list);
			}
		}


		void QContextLoggerTreeWidget::setContextVisibility(LoggerID id, bool isVisible)
		{
			const auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return;
			TreeData* treeData = it->second;
			treeData->setContextVisibility(isVisible);
		}
		bool QContextLoggerTreeWidget::getContextVisibility(LoggerID id) const
		{
			const auto& it = m_msgItems.find(id);
			if (it == m_msgItems.end())
				return false;
			TreeData* treeData = it->second;
			return treeData->getContextVisibility();
		}
		void QContextLoggerTreeWidget::setLevelVisibility(Level level, bool isVisible)
		{
			if(level >= sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]))
				return;
			m_levelVisibility[level] = isVisible;
			for (auto& context : m_msgItems)
			{
				TreeData* treeData = context.second;
				treeData->setLevelVisibility(level, isVisible);
			}
		}
		bool QContextLoggerTreeWidget::getLevelVisibility(Level level) const
		{
			if (level >= sizeof(m_levelVisibility) / sizeof(m_levelVisibility[0]))
				return false;
			return m_levelVisibility[level];
		}
		void QContextLoggerTreeWidget::onUpdateTimer()
		{
			// Periodic reconciliation to the bottom — catches range changes the
			// event-driven paths miss (row expansion, delayed layout).
			if (m_stickToBottom && !m_editorItem && m_treeWidget->isVisible())
				scrollToBottomGuarded();

			if (!m_messageCountDirty)
				return;

			for (auto& it : m_msgItems)
			{
				if (it.second->getParent() != nullptr)
					continue;

				unsigned int count = 0;
				it.second->updateMessageCount(count);
			}
			m_messageCountDirty = false;
		}
		void QContextLoggerTreeWidget::updateMessageCount(unsigned int& countOut)
		{
			for (auto& it : m_msgItems)
			{
				if (it.second->getParent() != nullptr)
					continue;

				unsigned int tmp = 0;
				it.second->updateMessageCount(tmp);
				countOut += tmp;
			}
		}
		void QContextLoggerTreeWidget::updateDateTimeFilter()
		{
			for (auto& it : m_msgItems)
			{
				TreeData* treeData = it.second;
				treeData->updateDateTimeFilter(m_dateTimeFilter);
			}
		}



		QContextLoggerTreeWidget::TreeData::TreeData(QContextLoggerTreeWidget* root, const LogObject::Info& info)
			: parent(nullptr)
		{
			this->root = root;
			msgItems.reserve(1024);

			childRoot = new QTreeWidgetItem(root->m_treeWidget);
			thisMessagesRoot = new QTreeWidgetItem(childRoot);

			this->loggerID = info.id;
			this->m_info = info;

			setupChildRoot();
			setupMessageRoot();
		}
		QContextLoggerTreeWidget::TreeData::TreeData(QContextLoggerTreeWidget* root, TreeData* parent, const LogObject::Info& info)
			: parent(parent)
		{
			this->root = root;
			msgItems.reserve(1024);
			if (parent)
			{
				childRoot = new QTreeWidgetItem(parent->childRoot);
			}
			else
			{
				childRoot = new QTreeWidgetItem(root->m_treeWidget);
			}
			thisMessagesRoot = new QTreeWidgetItem(childRoot);

			this->loggerID = info.id;
			this->m_info = info;

			setupChildRoot();
			setupMessageRoot();
		}
		QContextLoggerTreeWidget::TreeData::~TreeData()
		{
			std::vector<TreeData*> _children = children;
			children.clear();
			for (auto& it : _children)
			{
				it->parent = nullptr;
				delete it;
			}
			if (parent)
			{
				const auto& it = std::find(parent->children.begin(), parent->children.end(), this);
				if (it != parent->children.end())
					parent->children.erase(it);
			}
			if (root)
			{
				const auto& it = root->m_msgItems.find(loggerID);
				if (it != root->m_msgItems.end())
					root->m_msgItems.erase(it);
			}
			QTreeWidgetItem *childRoot_ = childRoot;
			childRoot = nullptr;
			thisMessagesRoot = nullptr;
			//detachLogger(*logger);
			delete childRoot_;
		}
		void QContextLoggerTreeWidget::TreeData::setupChildRoot()
		{
			const LogObject::Info& info = m_info;
			m_contextColor = info.color.toQColor();
			m_messageBackgroundColor = (info.color * 0.5f).toQColor();
			childRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, info.name.c_str());
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, info.creationTime.toString(root->m_timeFormat).c_str());
			childRoot->setBackground((int)HeaderPos::contextName, m_contextColor);
			childRoot->setBackground((int)HeaderPos::timestamp, m_contextColor);
			childRoot->setBackground((int)HeaderPos::message, m_contextColor);
		}
		void QContextLoggerTreeWidget::TreeData::setupMessageRoot()
		{
			thisMessagesRoot->setData((int)HeaderPos::contextName, Qt::DisplayRole, "Messages");
			thisMessagesRoot->setBackground((int)HeaderPos::contextName, m_contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::timestamp, m_contextColor);
			thisMessagesRoot->setBackground((int)HeaderPos::message, m_contextColor);
		}
		void QContextLoggerTreeWidget::TreeData::updateDateTime()
		{
			childRoot->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m_info.creationTime.toString(root->m_timeFormat).c_str());
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				//const Message& m = logger->getMessages()[i];
				//msgItems[i]->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(parent->m_timeFormat).c_str());

				msgItems[i].item->setData((int)HeaderPos::timestamp, Qt::DisplayRole, msgItems[i].msg.getDateTime().toString(root->m_timeFormat).c_str());
			}
		}

		void QContextLoggerTreeWidget::TreeData::onNewMessage(const Message& m)
		{
			QTreeWidgetItem* line = new QTreeWidgetItem(thisMessagesRoot);
			// Editable flag lets the read-only editor open for in-cell text selection.
			line->setFlags(line->flags() | Qt::ItemIsEditable);
			line->setData((int)HeaderPos::timestamp, Qt::DisplayRole, m.getDateTime().toString(root->m_timeFormat).c_str());
			line->setData((int)HeaderPos::message, Qt::DisplayRole, QString::fromStdString(m.getText()));

			line->setIcon((int)HeaderPos::contextName, Utilities::getIcon(m.getLevel()));
			line->setForeground((int)HeaderPos::message, m.getColor().toQColor());
			QFont font = line->font((int)HeaderPos::message);
			font.setBold(true);
			line->setFont((int)HeaderPos::message, font);

			line->setToolTip((int)HeaderPos::message, m.getText().c_str());
			line->setToolTip((int)HeaderPos::timestamp, m.getDateTime().toString(root->m_timeFormat).c_str());
			line->setToolTip((int)HeaderPos::contextName, m.getLevelString().c_str());

			//line->setBackgroundColor((int)HeaderPos::message, m.getColor().toQColor());
			line->setBackground((int)HeaderPos::contextName, m_messageBackgroundColor);
			line->setBackground((int)HeaderPos::timestamp, m_messageBackgroundColor);
			line->setBackground((int)HeaderPos::message, m_messageBackgroundColor);
			

			MessageData data;
			data.item = line;
			data.msg = m;

			unsigned int levelIndex = (unsigned int)m.getLevel();
			if (levelIndex < static_cast<unsigned int>(Level::__count))
			{
				if (!root->m_levelVisibility[levelIndex])
					data.setVisibilityFilter(MessageData::VisibilityBitMask::levelVisibility, false);
			}
			if (!root->m_dateTimeFilter.matches(data.msg.getDateTime()))
				data.setVisibilityFilter(MessageData::VisibilityBitMask::dateTimeVisibility, false);
			if (!root->matchesSearchText(data.msg.getText()))
				data.setVisibilityFilter(MessageData::VisibilityBitMask::textVisibility, false);

			msgItems.push_back(data);
		}
		QContextLoggerTreeWidget::TreeData* QContextLoggerTreeWidget::TreeData::createChild(const LogObject::Info& info)
		{
			TreeData *child = new TreeData(root, this, info);
			children.push_back(child);
			return child;
		}


		// Function to move a QTreeWidgetItem to a new parent
		static void changeParent(QTreeWidgetItem* item, QTreeWidgetItem* newParent) {
			if (item == nullptr || newParent == nullptr) return;

			// Get the current parent
			QTreeWidgetItem* currentParent = item->parent();

			if (currentParent) {
				// If the item has a parent, remove it from that parent
				currentParent->takeChild(currentParent->indexOfChild(item));
			}
			else {
				// If the item is a top-level item, remove it from the QTreeWidget directly
				QTreeWidget* treeWidget = item->treeWidget();
				if (treeWidget) {
					treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(item));
				}
			}

			// Add the item to the new parent
			newParent->addChild(item);
		}
		void QContextLoggerTreeWidget::TreeData::setParent(TreeData* newParent)
		{
			auto old_parent = childRoot->parent();
			if (old_parent != NULL) 
			{ 
				auto ix = old_parent->indexOfChild(childRoot);
				auto item_without_parent = old_parent->takeChild(ix); 
				newParent->childRoot->addChild(item_without_parent); 
			}
			else
			{
				//newParent->childRoot->addChild(childRoot);
				
			}



			if (parent)
			{
				const auto& it = std::find(parent->children.begin(), parent->children.end(), this);
				if (it != parent->children.end())
					parent->children.erase(it);
			}
			changeParent(childRoot, newParent->childRoot);
			parent = newParent;
			if (parent)
			{
				parent->children.push_back(this);
				//parent->childRoot->addChild(childRoot);
			}
		}
		/*void QContextLoggerTreeWidget::TreeData::changeParent(LoggerID childID, TreeData* newParent)
		{
			if(!newParent)
				return;
			for(size_t i=0; i<children.size(); ++i)
			{
				if(children[i]->loggerID == childID)
				{
					TreeData* child = children[i];
					children.erase(children.begin() + i);
					child->parent = newParent;
					newParent->children.push_back(child);
					return;
				}
			}
		}*/
		void QContextLoggerTreeWidget::TreeData::getChildLoggerIDsRecursive(std::vector<LoggerID>& list) const
		{
			for (auto& it : children)
			{
				it->getLoggerIDsRecursive(list);
			}
		}
		void QContextLoggerTreeWidget::TreeData::getLoggerIDsRecursive(std::vector<LoggerID>& list) const
		{
			list.push_back(loggerID);
			for (auto& it : children)
			{
				it->getLoggerIDsRecursive(list);
			}
		}

		void QContextLoggerTreeWidget::TreeData::setContextVisibility(bool isVisible)
		{
			childRoot->setHidden(!isVisible);
		}
		bool QContextLoggerTreeWidget::TreeData::getContextVisibility() const
		{
			return !childRoot->isHidden();
		}
		void QContextLoggerTreeWidget::TreeData::setLevelVisibility(Level level, bool isVisible)
		{
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				if (msgItems[i].msg.getLevel() == level)
				{
					msgItems[i].setVisibilityFilter(MessageData::VisibilityBitMask::levelVisibility, isVisible);
				}
			}
		}
		void QContextLoggerTreeWidget::TreeData::updateMessageCount(unsigned int& countOut)
		{
			for(auto &it : children)
			{
				unsigned int tmp = 0;
				it->updateMessageCount(tmp);
				countOut += tmp;
			}
			countOut += msgItems.size();
			QString childsCountTxt = "[" + QString::number(countOut) + "] Messages";
			childRoot->setData((int)HeaderPos::message, Qt::DisplayRole, childsCountTxt);

			QString messageCountTxt = "[" + QString::number(msgItems.size()) + "] Messages";
			thisMessagesRoot->setData((int)HeaderPos::message, Qt::DisplayRole, messageCountTxt);
		}
		void QContextLoggerTreeWidget::TreeData::clearMessages()
		{
			if (thisMessagesRoot)
			{
				auto childrenItems = thisMessagesRoot->takeChildren();
				for (QTreeWidgetItem* item : childrenItems)
					delete item;
			}
			msgItems.clear();
		}
		void QContextLoggerTreeWidget::TreeData::clearMessagesRecursive()
		{
			clearMessages();
			for(auto &it : children)
			{
				it->clearMessagesRecursive();
			}
		}
		//bool QContextLoggerTreeWidget::TreeData::getLoggerIsAlive() const
		//{
		//	return MetaInfo->isAlive;
		//}
		QContextLoggerTreeWidget::TreeData* QContextLoggerTreeWidget::TreeData::getParent() const
		{
			return parent;
		}
		void QContextLoggerTreeWidget::TreeData::updateDateTimeFilter(const DateTimeFilter& filter)
		{
			
			if (filter.active)
			{
				for (size_t i = 0; i < msgItems.size(); ++i)
				{
					bool elementIsVisible = true;
					MessageData &msgItem = msgItems[i];
					elementIsVisible = filter.matches(msgItem.msg.getDateTime());
					//elementIsVisible = 0;
					msgItem.setVisibilityFilter(MessageData::VisibilityBitMask::dateTimeVisibility, elementIsVisible);
				}
			}
			else
			{
				for (size_t i = 0; i < msgItems.size(); ++i)
				{
					msgItems[i].setVisibilityFilter(MessageData::VisibilityBitMask::dateTimeVisibility, true);
				}
			}
		}
		void QContextLoggerTreeWidget::TreeData::applyTextFilter(const std::function<bool(const std::string&)>& matcher)
		{
			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				const bool visible = matcher(msgItems[i].msg.getText());
				msgItems[i].setVisibilityFilter(MessageData::VisibilityBitMask::textVisibility, visible);
			}
		}
		void QContextLoggerTreeWidget::TreeData::saveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
		{
			std::vector<Message> messages;
			messages.reserve(msgItems.size());

			for (size_t i = 0; i < msgItems.size(); ++i)
			{
				if (msgItems[i].isVisible())
				{
					messages.push_back(msgItems[i].msg);
				}
			}
			list[loggerID] = messages;
		}

		std::vector<QTreeWidgetItem*> QContextLoggerTreeWidget::collectVisibleMessageItems() const
		{
			std::vector<QTreeWidgetItem*> out;
			for (const auto& kv : m_msgItems)
			{
				for (const auto& md : kv.second->msgItems)
				{
					if (md.isVisible() && md.item)
						out.push_back(md.item);
				}
			}
			return out;
		}
		int QContextLoggerTreeWidget::getMatchCount() const
		{
			if (m_searchText.isEmpty())
				return 0;
			int n = 0;
			for (const auto& kv : m_msgItems)
				for (const auto& md : kv.second->msgItems)
					if (md.isVisible())
						++n;
			return n;
		}
		void QContextLoggerTreeWidget::findNext(bool forward)
		{
			auto items = collectVisibleMessageItems();
			if (items.empty())
				return;
			// Sort visually — by their global row index in the tree.
			std::sort(items.begin(), items.end(), [this](QTreeWidgetItem* a, QTreeWidgetItem* b)
			{
				return m_treeWidget->visualItemRect(a).y() < m_treeWidget->visualItemRect(b).y();
			});
			QTreeWidgetItem* cur = m_treeWidget->currentItem();
			int idx = -1;
			for (size_t i = 0; i < items.size(); ++i)
				if (items[i] == cur) { idx = static_cast<int>(i); break; }
			int nextIdx;
			if (forward) nextIdx = (idx + 1) % static_cast<int>(items.size());
			else         nextIdx = (idx <= 0) ? static_cast<int>(items.size()) - 1 : idx - 1;
			QTreeWidgetItem* target = items[nextIdx];
			m_treeWidget->setCurrentItem(target);
			m_treeWidget->scrollToItem(target, QAbstractItemView::PositionAtCenter);
		}
		void QContextLoggerTreeWidget::showRowContextMenu(QTreeWidgetItem* item, const QPoint& globalPos)
		{
			if (!item)
				return;
			// Find the MessageData / logger ID for this item.
			LoggerID id = 0;
			QString msgText;
			for (const auto& kv : m_msgItems)
			{
				for (const auto& md : kv.second->msgItems)
				{
					if (md.item == item)
					{
						id = kv.first;
						msgText = QString::fromStdString(md.msg.getText());
						break;
					}
				}
				if (id != 0) break;
			}
			if (msgText.isEmpty())
				msgText = item->data((int)HeaderPos::message, Qt::DisplayRole).toString();

			QMenu menu;
			QAction* copyText = menu.addAction("Copy message text");
			menu.addSeparator();
			QAction* soloCtx = id != 0 ? menu.addAction("Solo this context") : nullptr;
			QAction* hideCtx = id != 0 ? menu.addAction("Hide this context") : nullptr;
			menu.addSeparator();
			QAction* hideLike = menu.addAction("Hide messages like this");
			QAction* chosen = menu.exec(globalPos);
			if (!chosen) return;
			if (chosen == copyText)
				QApplication::clipboard()->setText(msgText);
			else if (soloCtx && chosen == soloCtx)
				emit requestSoloContext(id);
			else if (hideCtx && chosen == hideCtx)
				emit requestHideContext(id);
			else if (chosen == hideLike)
				emit requestHideMessagesLike(msgText);
		}
	}
}
#endif
