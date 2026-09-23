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
					// item may be null (empty space) — showRowContextMenu still
					// needs to open so the "Show hidden logger" submenu is
					// reachable when nothing else is visible to right-click.
					QTreeWidgetItem* item = m_treeWidget->itemAt(pos);
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

			m_knownInfos[newContext.id] = newContext;

			// Invisible: never materialized, never redirected — its direct
			// messages are simply dropped in onNewMessage.
			if (newContext.visibilityPolicy == ReceiverVisibilityPolicy::Invisible)
				return;

			// FlattenSuggested (honored): fold into the nearest materialized
			// ancestor instead of getting its own node. If no materialized
			// ancestor exists yet (e.g. a root logger), it's only a suggestion —
			// fall through and materialize normally so no message is ever lost.
			if (newContext.contextDisplayPolicy == ContextDisplayPolicy::FlattenSuggested &&
				m_respectFlattenSuggestions)
			{
				const LoggerID target = resolveTreeParent(newContext.parentId);
				if (target != 0)
				{
					m_redirectTarget[newContext.id] = target;
					return;
				}
			}

			// Materialize normally, parented via resolveTreeParent (which — for
			// an all-AutoVisible/OwnContext hierarchy — resolves to exactly the
			// immediate parent, same as before this ancestor-walk existed).
			const LoggerID parentID = resolveTreeParent(newContext.parentId);
			TreeData* treeData = nullptr;
			if (parentID != 0)
			{
				TreeData* parentTreeData = m_msgItems.find(parentID)->second;
				treeData = parentTreeData->createChild(newContext);
			}
			else
			{
				treeData = new TreeData(this, newContext);
			}
			m_msgItems[newContext.id] = treeData;

			if (newContext.visibilityPolicy == ReceiverVisibilityPolicy::ManualAdd)
				treeData->setContextVisibility(false);
		}
		void QContextLoggerTreeWidget::onNewMessage(const Message& m)
		{
			const auto &it = m_msgItems.find(m.getLoggerID());
			if (it != m_msgItems.end())
			{
				it->second->onNewMessage(m);
				m_messageCountDirty = true;
				return;
			}
			const auto& redirectIt = m_redirectTarget.find(m.getLoggerID());
			if (redirectIt != m_redirectTarget.end())
			{
				const auto& targetIt = m_msgItems.find(redirectIt->second);
				if (targetIt != m_msgItems.end())
				{
					targetIt->second->onNewMessage(m);
					m_messageCountDirty = true;
				}
			}
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
			// LogManager's reparent path emits onChangeParent, not
			// onLoggerInfoChanged, so this widget has to track the parent
			// change itself before reconciling.
			const auto& knownIt = m_knownInfos.find(childID);
			if (knownIt != m_knownInfos.end())
				knownIt->second.parentId = parentID;

			reconcileLoggerPlacement(childID);
			m_messageCountDirty = true;
		}
		void QContextLoggerTreeWidget::onLoggerInfoChanged(const LogObject::Info& info)
		{
			m_knownInfos[info.id] = info;

			const auto& it = m_msgItems.find(info.id);
			if (it != m_msgItems.end())
				it->second->updateInfo(info);

			reconcileLoggerPlacement(info.id);
			m_messageCountDirty = true;
		}
		void QContextLoggerTreeWidget::setRespectFlattenSuggestions(bool respect)
		{
			if (m_respectFlattenSuggestions == respect)
				return;
			m_respectFlattenSuggestions = respect;
			for (const auto& kv : m_knownInfos)
				reconcileLoggerPlacement(kv.first);
			m_messageCountDirty = true;
		}
		LoggerID QContextLoggerTreeWidget::resolveTreeParent(LoggerID startParentId) const
		{
			LoggerID current = startParentId;
			int guard = 0;
			while (current != 0 && guard++ < 4096)
			{
				if (m_msgItems.find(current) != m_msgItems.end())
					return current;
				const auto& it = m_knownInfos.find(current);
				if (it == m_knownInfos.end())
					return 0;
				current = it->second.parentId;
			}
			return 0;
		}
		void QContextLoggerTreeWidget::reconcileLoggerPlacement(LoggerID id)
		{
			const auto infoIt = m_knownInfos.find(id);
			if (infoIt == m_knownInfos.end())
				return;
			const LogObject::Info& info = infoIt->second;

			auto findTreeData = [this](LoggerID lid) -> TreeData*
			{
				if (lid == 0)
					return nullptr;
				const auto it = m_msgItems.find(lid);
				return (it != m_msgItems.end()) ? it->second : nullptr;
			};

			const bool invisible = (info.visibilityPolicy == ReceiverVisibilityPolicy::Invisible);
			const bool flattenSuggested = (info.contextDisplayPolicy == ContextDisplayPolicy::FlattenSuggested) &&
				m_respectFlattenSuggestions;
			// Only relevant (and only computed) when it can actually suppress
			// materialization — flatten never suppresses materialization if no
			// ancestor resolves.
			const LoggerID flattenTarget = (!invisible && flattenSuggested) ? resolveTreeParent(info.parentId) : 0;
			const bool shouldMaterialize = !invisible && !(flattenSuggested && flattenTarget != 0);

			const auto curIt = m_msgItems.find(id);
			TreeData* currentTreeData = (curIt != m_msgItems.end()) ? curIt->second : nullptr;
			const bool isMaterialized = (currentTreeData != nullptr);

			const auto redirIt = m_redirectTarget.find(id);
			const bool wasRedirected = (redirIt != m_redirectTarget.end());
			const LoggerID oldRedirectTarget = wasRedirected ? redirIt->second : 0;

			if (isMaterialized && shouldMaterialize)
			{
				// materialized -> materialized: only re-point the tree parent if
				// a further-up ancestor's materialization changed; never touch an
				// already-user-toggled contextVisibility.
				const LoggerID newParentTarget = resolveTreeParent(info.parentId);
				TreeData* curParentTD = currentTreeData->getParent();
				const LoggerID curParentId = curParentTD ? curParentTD->loggerID : 0;
				if (newParentTarget != curParentId)
					currentTreeData->setParent(findTreeData(newParentTarget));
				return;
			}

			if (isMaterialized && !shouldMaterialize)
			{
				// materialized -> unmaterialized (demote: policy flipped to
				// Invisible, or flatten now applies).
				const LoggerID newTarget = resolveTreeParent(info.parentId);
				TreeData* newTargetTD = findTreeData(newTarget);

				// Re-home every direct child onto the new resolved target (or
				// promote to top-level, if none).
				std::vector<TreeData*> childrenCopy = currentTreeData->children;
				for (TreeData* childTD : childrenCopy)
					childTD->setParent(newTargetTD);

				// Re-home (or drop, if no target) every redirect pointing at this id.
				std::vector<LoggerID> redirectsToId;
				for (const auto& kv : m_redirectTarget)
					if (kv.second == id)
						redirectsToId.push_back(kv.first);
				for (LoggerID redirectedId : redirectsToId)
				{
					if (newTargetTD)
					{
						currentTreeData->migrateMessagesFor(redirectedId, newTargetTD);
						m_redirectTarget[redirectedId] = newTarget;
					}
					else
					{
						m_redirectTarget.erase(redirectedId);
					}
				}

				if (!invisible && flattenSuggested && newTargetTD)
				{
					// Demoting to flatten: migrate this node's own messages to
					// the new target and register a redirect.
					currentTreeData->migrateMessagesFor(id, newTargetTD);
					m_redirectTarget[id] = newTarget;
				}
				// Demoting to Invisible: its own messages are simply dropped
				// (not migrated) below, along with the TreeData.

				delete currentTreeData;
				return;
			}

			if (!isMaterialized && shouldMaterialize)
			{
				// unmaterialized -> materialized: create the TreeData.
				const LoggerID parentTarget = resolveTreeParent(info.parentId);
				TreeData* parentTD = findTreeData(parentTarget);
				TreeData* newTreeData = parentTD ? parentTD->createChild(info) : new TreeData(this, info);
				m_msgItems[id] = newTreeData;

				if (wasRedirected)
				{
					// Pull its already-forwarded messages back.
					TreeData* oldTargetTD = findTreeData(oldRedirectTarget);
					if (oldTargetTD)
						oldTargetTD->migrateMessagesFor(id, newTreeData);
					m_redirectTarget.erase(id);
				}

				// Other already-known loggers may have previously resolved
				// *past* id (because it wasn't materialized yet) straight to
				// parentTarget — the same ancestor id itself just resolved
				// to. Now that id sits in m_msgItems, anything whose own
				// resolution walk would now stop at id instead needs to be
				// re-pointed at it. This only ever affects nodes currently
				// parented at exactly parentTarget (one level): a node's
				// placement depends only on its nearest materialized
				// ancestor, so if that ancestor isn't parentTarget, id's
				// appearance elsewhere can't change it — and if a node's
				// chain does pass through id and used to resolve past it to
				// parentTarget, that node's current tree-parent is
				// necessarily parentTarget itself, never something deeper
				// (the same one-level property the demote branch above
				// already relies on).
				std::vector<TreeData*> candidates;
				if (parentTD)
				{
					for (TreeData* child : parentTD->children)
						if (child != newTreeData)
							candidates.push_back(child);
				}
				else
				{
					for (const auto& kv : m_msgItems)
						if (kv.second != newTreeData && kv.second->getParent() == nullptr)
							candidates.push_back(kv.second);
				}
				for (TreeData* candidate : candidates)
				{
					const auto candInfoIt = m_knownInfos.find(candidate->loggerID);
					if (candInfoIt == m_knownInfos.end())
						continue;
					if (resolveTreeParent(candInfoIt->second.parentId) == id)
						candidate->setParent(newTreeData);
				}

				// Same idea for loggers currently flatten-redirected straight
				// past id to parentTarget.
				std::vector<LoggerID> redirectsToParentTarget;
				for (const auto& kv : m_redirectTarget)
					if (kv.second == parentTarget)
						redirectsToParentTarget.push_back(kv.first);
				for (LoggerID redirectedId : redirectsToParentTarget)
				{
					const auto redirInfoIt = m_knownInfos.find(redirectedId);
					if (redirInfoIt == m_knownInfos.end())
						continue;
					if (resolveTreeParent(redirInfoIt->second.parentId) != id)
						continue;
					if (parentTD)
						parentTD->migrateMessagesFor(redirectedId, newTreeData);
					m_redirectTarget[redirectedId] = id;
				}

				// Force-hide only here if ManualAdd — never re-hide something
				// already materialized and revealed.
				if (info.visibilityPolicy == ReceiverVisibilityPolicy::ManualAdd)
					newTreeData->setContextVisibility(false);
				return;
			}

			// unmaterialized -> unmaterialized.
			if (invisible)
			{
				// Flips from flatten-redirected to Invisible: just drop the
				// redirect entry — already-forwarded messages stay where they
				// are, consistent with how enabled-toggling never retroactively
				// erases history.
				if (wasRedirected)
					m_redirectTarget.erase(id);
				return;
			}
			// Still flatten-suggested-and-honored with a resolved target.
			if (wasRedirected)
			{
				if (oldRedirectTarget != flattenTarget)
				{
					TreeData* oldTargetTD = findTreeData(oldRedirectTarget);
					TreeData* newTargetTD = findTreeData(flattenTarget);
					if (oldTargetTD && newTargetTD)
						oldTargetTD->migrateMessagesFor(id, newTargetTD);
					m_redirectTarget[id] = flattenTarget;
				}
			}
			else if (flattenTarget != 0)
			{
				m_redirectTarget[id] = flattenTarget;
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


		// Function to move a QTreeWidgetItem to a new parent. newParent == nullptr
		// promotes the item to a top-level item of its own QTreeWidget instead.
		static void changeParent(QTreeWidgetItem* item, QTreeWidgetItem* newParent) {
			if (item == nullptr) return;

			// Get the current parent, and the owning tree widget (captured before
			// detaching — an item briefly untracked by any parent can still
			// resolve it via treeWidget()).
			QTreeWidgetItem* currentParent = item->parent();
			QTreeWidget* treeWidget = item->treeWidget();

			if (currentParent) {
				// If the item has a parent, remove it from that parent
				currentParent->takeChild(currentParent->indexOfChild(item));
			}
			else if (treeWidget) {
				// If the item is a top-level item, remove it from the QTreeWidget directly
				treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(item));
			}

			if (newParent) {
				// Add the item to the new parent
				newParent->addChild(item);
			}
			else if (treeWidget) {
				// No new parent: (re)promote to a top-level item.
				treeWidget->addTopLevelItem(item);
			}
		}
		void QContextLoggerTreeWidget::TreeData::setParent(TreeData* newParent)
		{
			// Refuse to become a child of our own descendant. Qt does not
			// survive a QTreeWidgetItem cycle (ISS-006). LogManager rejects
			// cyclic reparenting for live loggers, but messages loaded from a
			// file are replayed straight into the views and bypass it.
			for (TreeData* ancestor = newParent; ancestor; ancestor = ancestor->parent)
			{
				if (ancestor == this)
					return;
			}
			if (parent)
			{
				const auto& it = std::find(parent->children.begin(), parent->children.end(), this);
				if (it != parent->children.end())
					parent->children.erase(it);
			}
			// newParent == nullptr promotes this to a top-level item.
			changeParent(childRoot, newParent ? newParent->childRoot : nullptr);
			parent = newParent;
			if (parent)
			{
				parent->children.push_back(this);
			}
		}
		void QContextLoggerTreeWidget::TreeData::updateInfo(const LogObject::Info& info)
		{
			m_info = info;
			// Recomputes m_contextColor/m_messageBackgroundColor and refreshes
			// the displayed name/timestamp/backgrounds from the new Info.
			setupChildRoot();
			setupMessageRoot();
		}
		void QContextLoggerTreeWidget::TreeData::migrateMessagesFor(LoggerID sourceLoggerID, TreeData* destination)
		{
			if (!destination || destination == this)
				return;
			for (size_t i = 0; i < msgItems.size(); )
			{
				if (msgItems[i].msg.getLoggerID() == sourceLoggerID)
				{
					changeParent(msgItems[i].item, destination->thisMessagesRoot);
					destination->msgItems.push_back(msgItems[i]);
					msgItems.erase(msgItems.begin() + i);
				}
				else
				{
					++i;
				}
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
		std::vector<LoggerID> QContextLoggerTreeWidget::getManuallyHiddenLoggerIds() const
		{
			std::vector<LoggerID> result;
			for (const auto& kv : m_msgItems)
			{
				TreeData* treeData = kv.second;
				if (treeData->m_info.visibilityPolicy == ReceiverVisibilityPolicy::ManualAdd &&
					!treeData->getContextVisibility())
					result.push_back(kv.first);
			}
			return result;
		}
		void QContextLoggerTreeWidget::showRowContextMenu(QTreeWidgetItem* item, const QPoint& globalPos)
		{
			// item may be null (empty space) — the menu still needs to open so
			// the "Show hidden logger" submenu is reachable when nothing else
			// is visible to right-click on.

			// Find the MessageData / logger ID for this item.
			LoggerID id = 0;
			QString msgText;
			if (item)
			{
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
			}

			QMenu menu;
			QAction* copyText = item ? menu.addAction("Copy message text") : nullptr;
			if (item)
				menu.addSeparator();
			QAction* soloCtx = id != 0 ? menu.addAction("Solo this context") : nullptr;
			QAction* hideCtx = id != 0 ? menu.addAction("Hide this context") : nullptr;
			QAction* hideLike = item ? menu.addAction("Hide messages like this") : nullptr;

			const std::vector<LoggerID> hiddenIds = getManuallyHiddenLoggerIds();
			std::unordered_map<QAction*, LoggerID> showActions;
			if (!hiddenIds.empty())
			{
				menu.addSeparator();
				QMenu* showHiddenMenu = menu.addMenu("Show hidden logger");
				for (LoggerID hiddenId : hiddenIds)
				{
					const auto& it = m_msgItems.find(hiddenId);
					const QString label = (it != m_msgItems.end())
						? QString::fromStdString(it->second->m_info.name)
						: QString::number(hiddenId);
					QAction* act = showHiddenMenu->addAction(label);
					showActions[act] = hiddenId;
				}
			}

			if (menu.isEmpty())
				return;
			QAction* chosen = menu.exec(globalPos);
			if (!chosen) return;
			if (copyText && chosen == copyText)
				QApplication::clipboard()->setText(msgText);
			else if (soloCtx && chosen == soloCtx)
				emit requestSoloContext(id);
			else if (hideCtx && chosen == hideCtx)
				emit requestHideContext(id);
			else if (hideLike && chosen == hideLike)
				emit requestHideMessagesLike(msgText);
			else
			{
				const auto& showIt = showActions.find(chosen);
				if (showIt != showActions.end())
					emit requestShowLogger(showIt->second);
			}
		}
	}
}
#endif
