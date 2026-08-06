#include "ui/Widgets/QAbstractLogWidget.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogWidget.h"
#include "Utilities/Resources.h"
#include "Utilities/Export.h"
#include "Utilities/Import.h"
#include <QTreeWidget>
#include <QMetaType>
#include <QSplitter>
#include <QCheckBox>
#include <QFileDialog>
#include "ui/Widgets/DateTimeWidget.h"
#include <fstream>
#include <QApplication>
#include <QThread>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QTextBrowser>
#include <QSplitter>
#include <QFrame>


namespace Log
{
	namespace UIWidgets
	{
		QAbstractLogWidget::QAbstractLogWidget(QWidget* parent)
			: QWidget(parent)
			, ui(new Ui::QAbstractLogWidget)
		{
			ui->setupUi(this);
			qRegisterMetaType<QAbstractLogWidget::SubWidget>("QAbstractLogWidget::SubWidget");
			for (int i = 0; i < __featureCount; ++i)
				m_features[i] = true;
			ui->context_scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
			ui->logLevelContent_frame->layout()->setAlignment(Qt::AlignTop);
			ui->searchIcon_label->setPixmap(Resources::getIconSearch().pixmap(16, 16));

			m_autoCreateNewCheckBoxForNewContext = true;
			m_filterTextEdits = { ui->contextFilter_lineEdit };
			for (size_t i = 0; i < m_filterTextEdits.size(); ++i)
			{
				QObject::connect(m_filterTextEdits[i], &QLineEdit::textChanged,
					this, &QAbstractLogWidget::onFilterTextChangedSlot);
			}

			for (int i = 0; i < Level::__count; ++i)
			{
				QCheckBox* checkBox = new QCheckBox(parent);
				checkBox->setChecked(true);
				checkBox->setText(Utilities::getLevelStr((Level)i).c_str());
				checkBox->setIcon(Utilities::getIcon((Level)i));
				QObject::connect(checkBox, &QCheckBox::stateChanged,
					this, &QAbstractLogWidget::onLevelCheckBoxStateChangedSlot);
				m_levelCheckBoxes[i] = checkBox;
				if (ui->logLevelContent_frame->layout())
					ui->logLevelContent_frame->layout()->addWidget(checkBox);
			}
			QObject::connect(ui->allContext_checkBox, &QCheckBox::stateChanged, this, &QAbstractLogWidget::onAllContextCheckBoxStateChanged);

			ui->dateTimeFilterActivate_checkBox->setChecked(false);
			connect(ui->dateTimeFilterActivate_checkBox, &QCheckBox::stateChanged,
				this, &QAbstractLogWidget::onDateTimeFilterActivate_checkBox_stateChanged);
			connect(ui->dateTimeFilterMin_dateTimeEdit, &DateTimeWidget::dateTimeChanged,
				this, &QAbstractLogWidget::onDateTimeFilterMin_changed);
			connect(ui->dateTimeFilterMax_dateTimeEdit, &DateTimeWidget::dateTimeChanged,
				this, &QAbstractLogWidget::onDateTimeFilterMax_changed);

			connect(ui->dateTimeFilterMinNow_pushButton, &QPushButton::pressed,
				this, &QAbstractLogWidget::onDateTimeFilterMinNow_pushButton_clicked);
			connect(ui->dateTimeFilterMaxNow_pushButton, &QPushButton::pressed,
				this, &QAbstractLogWidget::onDateTimeFilterMaxNow_pushButton_clicked);

			{
				QHBoxLayout* layout = new QHBoxLayout();
				layout->setAlignment(Qt::AlignCenter);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				layout->setMargin(0);
#else
				layout->setContentsMargins(0, 0, 0, 0);
#endif
				
				
				ui->dateTimeFilterMinNow_pushButton->setLayout(layout);
				QLabel* nowLabel = new QLabel();
				nowLabel->setPixmap(Resources::getIconReload().pixmap(20, 20));
				layout->addWidget(nowLabel);
				ui->dateTimeFilterMinNow_pushButton->setFixedSize(20, 20);
				ui->dateTimeFilterMinNow_pushButton->setText("");
			}
			{
				QHBoxLayout* layout = new QHBoxLayout();
				layout->setAlignment(Qt::AlignCenter);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				layout->setMargin(0);
#else
				layout->setContentsMargins(0, 0, 0, 0);
#endif
				ui->dateTimeFilterMaxNow_pushButton->setLayout(layout);
				QLabel* nowLabel = new QLabel();
				nowLabel->setPixmap(Resources::getIconReload().pixmap(20, 20));
				layout->addWidget(nowLabel);
				ui->dateTimeFilterMaxNow_pushButton->setFixedSize(20, 20);
				ui->dateTimeFilterMaxNow_pushButton->setText("");
			}

			for (int i = 0; i < DateTime::Range::__count; ++i)
				ui->dateTimeFilterType_comboBox->addItem(DateTime::getRangeStr((DateTime::Range)i).c_str());
			ui->dateTimeFilterType_comboBox->setCurrentIndex(DateTime::Range::between);
			connect(ui->dateTimeFilterType_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
				this, &QAbstractLogWidget::onDateTimeFilterType_changed);

			// Size the log-level list to fit all level rows exactly (no scroll):
			// the scrollbar is disabled in the .ui and the frame uses a Maximum
			// vertical size policy, so pin the scroll area to its content height.
			if (auto* content = ui->logLevelContent_frame->layout())
			{
				const int h = content->sizeHint().height();
				ui->logLevel_scrollArea->setMinimumHeight(h);
				ui->logLevel_scrollArea->setMaximumHeight(h);
			}

			// Left column: only the context filter grows; the other three panels
			// stay at their content size. Context therefore absorbs all slack, so
			// no trailing spacer is needed.
			if (auto* col = qobject_cast<QVBoxLayout*>(ui->settings_frame->layout()))
			{
				col->setStretch(0, 0); // logLevel_frame
				col->setStretch(1, 1); // contextFilter_frame
				col->setStretch(2, 0); // dateTimeFilter_frame
				col->setStretch(3, 0); // edit_frame
			}

			// Persistent thin strip on the left that always holds the collapse /
			// expand toggle for the whole settings column.
			buildCollapseStrip();
		}
		QAbstractLogWidget::~QAbstractLogWidget()
		{

		}

		void QAbstractLogWidget::clear()
		{
			m_contextData.clear();
		}
		void QAbstractLogWidget::postConstructorInit()
		{
			std::vector< LogObject::Info> loggers = LogManager::getLogObjectsInfo();
			for (const auto& logger : loggers)
			{
				onNewLogger(logger);
			}

			// The settings column starts EXPANDED; app code may collapse it via
			// setSubWidgetCollapsed(settingsFrame, true).
			applySettingsCollapse();
		}

		void QAbstractLogWidget::buildCollapseStrip()
		{
			if (m_collapseStrip)
				return;

			// A persistent ~24px strip inserted as the leftmost splitter child.
			// It always holds the toggle button, so the settings column can be
			// re-expanded even while collapsed.
			static const int kStripWidth = 24;
			m_collapseStrip = new QFrame(this);
			m_collapseStrip->setFrameShape(QFrame::NoFrame);
			m_collapseStrip->setFixedWidth(kStripWidth);

			QVBoxLayout* stripLayout = new QVBoxLayout(m_collapseStrip);
			stripLayout->setContentsMargins(1, 1, 1, 1);
			stripLayout->setAlignment(Qt::AlignTop);

			m_collapseToggleButton = new QToolButton(m_collapseStrip);
			m_collapseToggleButton->setAutoRaise(true);
			m_collapseToggleButton->setFocusPolicy(Qt::NoFocus);
			m_collapseToggleButton->setArrowType(Qt::LeftArrow);
			m_collapseToggleButton->setToolTip(tr("Collapse / expand the settings panel"));
			m_collapseToggleButton->setFixedSize(kStripWidth - 2, kStripWidth - 2);
			stripLayout->addWidget(m_collapseToggleButton);

			QObject::connect(m_collapseToggleButton, &QToolButton::clicked, this,
				[this]() { setSubWidgetCollapsed(SubWidget::settingsFrame, !m_settingsCollapsed); });

			// Place the strip immediately left of the settings column.
			if (ui->splitter)
			{
				ui->splitter->insertWidget(0, m_collapseStrip);
				ui->splitter->setStretchFactor(0, 0);
			}
		}

		void QAbstractLogWidget::setSubWidgetCollapsible(SubWidget /*widget*/, bool /*collapsible*/)
		{
			// Deprecated no-op. Only the whole settings column collapses now, and
			// it is always collapsible via its persistent strip toggle. Retained
			// for source compatibility with the former per-element API.
		}

		void QAbstractLogWidget::setSubWidgetCollapsed(SubWidget widget, bool collapsed)
		{
			// Only the settings column collapses. Per-element collapse was removed.
			if (widget != SubWidget::settingsFrame)
				return;
			m_settingsCollapsed = collapsed;
			applySettingsCollapse();
			emit subWidgetCollapsedChanged(SubWidget::settingsFrame, collapsed);
		}

		bool QAbstractLogWidget::isSubWidgetCollapsed(SubWidget widget) const
		{
			if (widget == SubWidget::settingsFrame)
				return m_settingsCollapsed;
			return false;
		}

		void QAbstractLogWidget::applySettingsCollapse()
		{
			// Strip is always visible while the settings area is enabled; the
			// settings column itself is hidden when collapsed so the console
			// (splitter neighbor) reclaims the freed width. Whole-frame enable
			// state stays orthogonal to collapse.
			if (m_collapseStrip)
				m_collapseStrip->setVisible(m_settingsEnabled);
			ui->settings_frame->setVisible(m_settingsEnabled && !m_settingsCollapsed);
			if (m_collapseToggleButton)
				m_collapseToggleButton->setArrowType(
					m_settingsCollapsed ? Qt::RightArrow : Qt::LeftArrow);
		}

		bool QAbstractLogWidget::saveVisibleMessages(const std::string& outputFile) const
		{
			std::unordered_map<LoggerID, std::vector<Message>> list;
			getSaveVisibleMessages(list);
			// Use the Info we already have on each context so file-loaded loggers
			// (not in the LogManager singleton) still export correctly.
			std::vector<LogObject::Info> infos;
			infos.reserve(m_contextData.size());
			for (const auto& kv : m_contextData)
				infos.push_back(kv.second.info);
			return Export::saveToFile(infos, list, outputFile);
		}
		bool QAbstractLogWidget::loadMessagesFromFile(const std::string& inputFile)
		{
			std::vector<std::pair<LogObject::Info, std::vector<Message>>> list;
			std::vector<std::pair<LoggerID, LoggerID>> reparents;
			if(Import::loadFromFile(list, reparents, inputFile))
			{
				clear();
				onMessagesLoadStarted();
				for (const auto& context : list)
				{
					onNewLogger(context.first);
					for (const auto& message : context.second)
					{
						onLogMessage(message);
					}
				}
				for (const auto& r : reparents)
				{
					onChangeParent(r.first, r.second);
				}
				onMessagesLoaded();
				return true;
			}
			return false;
		}

		void QAbstractLogWidget::setLevelEnabled(Level level, bool enable)
		{
			if (level >= Level::__count)
				return;
			if (m_levelCheckBoxes[level])
			{
				m_levelCheckBoxes[level]->setChecked(enable);
				onLevelCheckBoxChanged(level, level, m_levelCheckBoxes[level]->isChecked());
			}
		}

		void QAbstractLogWidget::disableSubWidget(SubWidget widget)
		{
			switch (widget)
			{
				case SubWidget::settingsFrame:
					// Hide the whole left area (strip + column); collapse state is
					// preserved and re-applied on enable.
					m_settingsEnabled = false;
					applySettingsCollapse();
					break;
				case SubWidget::logLevelFilter:
					ui->logLevel_frame->setVisible(false);
					break;
				case SubWidget::contextFilter:
					ui->contextFilter_frame->setVisible(false);
					break;
				case SubWidget::dateTimeFilter:
					ui->dateTimeFilter_frame->setVisible(false);
					break;
				case SubWidget::editFrame:
					ui->edit_frame->setVisible(false);
					break;
				case SubWidget::contentFrame:
					ui->content_frame->setVisible(false);
					break;
			}
		}
		void QAbstractLogWidget::enableSubWidget(SubWidget widget)
		{
			switch (widget)
			{
			case SubWidget::settingsFrame:
				// Restore the left area to its stored collapse state.
				m_settingsEnabled = true;
				applySettingsCollapse();
				break;
			case SubWidget::logLevelFilter:
				ui->logLevel_frame->setVisible(true);
				break;
			case SubWidget::contextFilter:
				ui->contextFilter_frame->setVisible(true);
				break;
			case SubWidget::dateTimeFilter:
				ui->dateTimeFilter_frame->setVisible(true);
				break;
			case SubWidget::editFrame:
				ui->edit_frame->setVisible(true);
				break;
			case SubWidget::contentFrame:
				ui->content_frame->setVisible(true);
				break;
			}
		}

		void QAbstractLogWidget::onAllContextCheckBoxStateChanged(int state)
		{
			if (m_ignoreAllContextCheckBox_signals)
				return;
			if (state == Qt::PartiallyChecked)
			{
				m_ignoreAllContextCheckBox_signals = true;
				ui->allContext_checkBox->setCheckState(Qt::Checked);
				m_ignoreAllContextCheckBox_signals = false;
			}
			bool isChecked = ui->allContext_checkBox->isChecked();
			for (auto& loggerData : m_contextData)
			{
				loggerData.second.checkBox->setChecked(isChecked);
			}
		}

		void QAbstractLogWidget::on_clear_pushButton_clicked()
		{
			clear();
		}
		void QAbstractLogWidget::on_save_pushButton_clicked()
		{
			QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Log Files (*.log);;All Files (*)"));
			if (fileName.isEmpty())
				return;
			saveVisibleMessages(fileName.toStdString());
		}

		void QAbstractLogWidget::onLevelCheckBoxStateChangedSlot(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (size_t i = 0; i < (size_t)Level::__count; ++i)
			{
				if (m_levelCheckBoxes[i] == checkBox)
				{
					onLevelCheckBoxChanged(i, (Level)i, state == Qt::Checked);
					break;
				}
			}
		}
		void QAbstractLogWidget::onFilterTextChangedSlot(const QString& text)
		{
			QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
			if (lineEdit == nullptr)
				return;

			for (size_t i = 0; i < m_filterTextEdits.size(); ++i)
			{
				if (m_filterTextEdits[i] == lineEdit)
				{
					onFilterTextChanged(i, lineEdit, text.toStdString());
					break;
				}
			}
		}
		void QAbstractLogWidget::onCheckBoxStateChangedSlot(int state)
		{
			LOGGER_UNUSED(state);
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (const auto& contextData : m_contextData)
			{
				if (contextData.second.checkBox == checkBox)
				{
					onContextCheckBoxChanged(contextData.second, checkBox->isChecked());
					break;
				}
			}
		}

		void QAbstractLogWidget::onDateTimeFilterActivate_checkBox_stateChanged(int state)
		{
			LOGGER_UNUSED(state);
			if (ui->dateTimeFilterActivate_checkBox->isChecked())
			{
				ui->dateTimeFilterMax_dateTimeEdit->setEnabled(true);
				ui->dateTimeFilterMin_dateTimeEdit->setEnabled(true);
				m_dateTimeFilter.active = true;
				m_dateTimeFilter.min = ui->dateTimeFilterMin_dateTimeEdit->getDateTime();
				m_dateTimeFilter.max = ui->dateTimeFilterMax_dateTimeEdit->getDateTime();
				m_dateTimeFilter.rangeType = (DateTime::Range)ui->dateTimeFilterType_comboBox->currentIndex();
			}
			else
			{
				m_dateTimeFilter.active = false;
				ui->dateTimeFilterMax_dateTimeEdit->setEnabled(false);
				ui->dateTimeFilterMin_dateTimeEdit->setEnabled(false);
			}
			onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogWidget::onDateTimeFilterMin_changed(const DateTime& dateTime)
		{
			m_dateTimeFilter.min = dateTime;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogWidget::onDateTimeFilterMax_changed(const DateTime& dateTime)
		{
			m_dateTimeFilter.max = dateTime;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogWidget::onDateTimeFilterMinNow_pushButton_clicked()
		{
			ui->dateTimeFilterMin_dateTimeEdit->setNow();
			m_dateTimeFilter.min = ui->dateTimeFilterMin_dateTimeEdit->getDateTime();
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogWidget::onDateTimeFilterMaxNow_pushButton_clicked()
		{
			ui->dateTimeFilterMax_dateTimeEdit->setNow();
			m_dateTimeFilter.max = ui->dateTimeFilterMax_dateTimeEdit->getDateTime();
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}

		void QAbstractLogWidget::onDateTimeFilterType_changed(int index)
		{
			m_dateTimeFilter.rangeType = (DateTime::Range)index;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}





		void QAbstractLogWidget::onNewLogger(LogObject::Info loggerInfo)
		{
			// A logger can be delivered twice for pre-existing loggers: once by the
			// view's constructor replay (postConstructorInit) and once by the
			// SignalReceiver replay. Guard against duplicate rows/checkboxes.
			if (m_contextData.find(loggerInfo.id) != m_contextData.end())
				return;

			ContextData data;
			data.checkBox = new QCheckBox(this);
			QPalette p = data.checkBox->palette();
			data.checkBox->setAutoFillBackground(true);
			p.setColor(QPalette::Button, loggerInfo.color.toQColor());
			data.checkBox->setPalette(p);
			data.checkBox->setChecked(true);
			data.checkBox->setText(loggerInfo.name.c_str());
			data.id = loggerInfo.id;
			data.info = loggerInfo;
			QObject::connect(data.checkBox, &QCheckBox::stateChanged,
				this, &QAbstractLogWidget::onCheckBoxStateChangedSlot);
			m_contextData[loggerInfo.id] = data;
			ui->context_scrollAreaWidgetContents->layout()->addWidget(data.checkBox);
		}
		void QAbstractLogWidget::onLoggerInfoChanged(LogObject::Info info)
		{
			auto it = m_contextData.find(info.id);
			if (it != m_contextData.end())
				it->second.info = info;
		}
		void QAbstractLogWidget::onLogMessage(Message message)
		{
			LOGGER_UNUSED(message);
		}
		void QAbstractLogWidget::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			auto it = m_contextData.find(childID);
			if (it != m_contextData.end())
				it->second.info.parentId = newParentID;
		}



		void QAbstractLogWidget::setContentWidget(QWidget* widget)
		{
			auto* layout = ui->content_frame->layout();
			if (!m_searchLineEdit)
			{
				m_searchBarWidget = new QWidget(ui->content_frame);
				auto* h = new QHBoxLayout(m_searchBarWidget);
				h->setContentsMargins(2, 2, 2, 2);
				h->setSpacing(4);
				QLabel* lbl = new QLabel("Search:", m_searchBarWidget);
				m_searchLineEdit = new QLineEdit(m_searchBarWidget);
				m_searchLineEdit->setPlaceholderText("Filter messages (prefix with ! to exclude)");
				m_searchLineEdit->setClearButtonEnabled(true);
				m_findPrevButton = new QPushButton("▲", m_searchBarWidget);
				m_findPrevButton->setToolTip("Previous match (Shift+F3)");
				m_findPrevButton->setFixedWidth(24);
				m_findNextButton = new QPushButton("▼", m_searchBarWidget);
				m_findNextButton->setToolTip("Next match (F3)");
				m_findNextButton->setFixedWidth(24);
				m_matchCountLabel = new QLabel("0", m_searchBarWidget);
				m_matchCountLabel->setToolTip("Number of matches");
				m_matchCountLabel->setMinimumWidth(40);
				m_matchCountLabel->setAlignment(Qt::AlignCenter);
				m_searchRegexCheckBox = new QCheckBox(".*", m_searchBarWidget);
				m_searchRegexCheckBox->setToolTip("Interpret the search text as a regular expression");
				h->addWidget(lbl);
				h->addWidget(m_searchLineEdit, 1);
				h->addWidget(m_matchCountLabel);
				h->addWidget(m_findPrevButton);
				h->addWidget(m_findNextButton);
				h->addWidget(m_searchRegexCheckBox);
				QObject::connect(m_searchLineEdit, &QLineEdit::textChanged,
					this, &QAbstractLogWidget::onSearchLineEditChanged);
				QObject::connect(m_searchLineEdit, &QLineEdit::returnPressed,
					this, [this]() { findNext(true); });
				QObject::connect(m_searchRegexCheckBox, &QCheckBox::stateChanged,
					this, &QAbstractLogWidget::onSearchRegexToggled);
				QObject::connect(m_findPrevButton, &QPushButton::clicked,
					this, [this]() { findNext(false); });
				QObject::connect(m_findNextButton, &QPushButton::clicked,
					this, [this]() { findNext(true); });
				layout->addWidget(m_searchBarWidget);

				m_contentSplitter = new QSplitter(Qt::Vertical, ui->content_frame);
				m_contentSplitter->setChildrenCollapsible(false);
				if (auto* box = qobject_cast<QBoxLayout*>(layout))
					box->addWidget(m_contentSplitter, 1);
				else
					layout->addWidget(m_contentSplitter);

				auto* details = new QTextBrowser(m_contentSplitter);
				details->setOpenExternalLinks(false);
				details->setPlaceholderText("Details of the selected message will appear here.");
				details->setMinimumHeight(60);
				m_detailsPane = details;

				// Apply any feature flags that were set before setContentWidget ran.
				m_searchBarWidget->setVisible(m_features[SearchBar]);
				if (m_searchRegexCheckBox) m_searchRegexCheckBox->setVisible(m_features[SearchRegexCheckBox]);
				m_findPrevButton->setVisible(m_features[FindNextPrev]);
				m_findNextButton->setVisible(m_features[FindNextPrev]);
				m_matchCountLabel->setVisible(m_features[MatchCount]);
				m_detailsPane->setVisible(m_features[DetailsPane]);
			}
			// First widget goes above the details pane.
			int belowIdx = m_contentSplitter->indexOf(m_detailsPane);
			m_contentSplitter->insertWidget(belowIdx, widget);
			m_contentSplitter->setStretchFactor(belowIdx, 4);
			m_contentSplitter->setStretchFactor(belowIdx + 1, 1);
		}
		void QAbstractLogWidget::setDetailsHtml(const QString& html)
		{
			if (auto* tb = qobject_cast<QTextBrowser*>(m_detailsPane))
				tb->setHtml(html);
		}
		std::string QAbstractLogWidget::getContextNameFor(LoggerID id) const
		{
			auto it = m_contextData.find(id);
			if (it != m_contextData.end())
				return it->second.info.name;
			return {};
		}
		void QAbstractLogWidget::updateDetailsFor(const Message& msg, bool hasSelection)
		{
			if (!hasSelection)
			{
				setDetailsHtml(QString());
				return;
			}
			setDetailsHtml(formatMessageAsHtml(msg, getContextNameFor(msg.getLoggerID())));
		}
		QString QAbstractLogWidget::formatMessageAsHtml(const Message& msg, const std::string& contextName)
		{
			const QColor lvlColor = msg.getColor().toQColor();
			const QString lvlName = QString::fromStdString(Utilities::getLevelStr(msg.getLevel()));
			const QString ctx = contextName.empty()
				? QString("(logger #%1)").arg(msg.getLoggerID())
				: QString::fromStdString(contextName);
			const QDateTime dt = msg.getDateTime().toQDateTime();
			const QString tsHuman = dt.toString("yyyy-MM-dd hh:mm:ss.zzz");
			const qint64 tsEpoch = dt.toMSecsSinceEpoch();
			const QString textHtml = QString::fromStdString(msg.getText()).toHtmlEscaped().replace("\n", "<br>");
			return QString(
				"<b>Context:</b> %1 &nbsp; <b>ID:</b> %2<br>"
				"<b>Level:</b> <span style='color:%3'>%4</span><br>"
				"<b>Time:</b> %5 &nbsp; <span style='color:#888'>(%6 ms)</span>"
				"<hr>"
				"<pre style='white-space: pre-wrap;'>%7</pre>"
			).arg(ctx.toHtmlEscaped())
			 .arg(msg.getLoggerID())
			 .arg(lvlColor.name())
			 .arg(lvlName)
			 .arg(tsHuman)
			 .arg(tsEpoch)
			 .arg(textHtml);
		}

		void QAbstractLogWidget::setFeatureEnabled(Feature f, bool enabled)
		{
			if (f < 0 || f >= __featureCount)
				return;
			m_features[f] = enabled;
			if (!m_searchBarWidget)
				return; // widgets built lazily; effect applies on first paint
			switch (f)
			{
			case SearchBar:
				m_searchBarWidget->setVisible(enabled);
				break;
			case SearchRegexCheckBox:
				if (m_searchRegexCheckBox) m_searchRegexCheckBox->setVisible(enabled);
				break;
			case FindNextPrev:
				if (m_findPrevButton) m_findPrevButton->setVisible(enabled);
				if (m_findNextButton) m_findNextButton->setVisible(enabled);
				break;
			case MatchCount:
				if (m_matchCountLabel) m_matchCountLabel->setVisible(enabled);
				break;
			case DetailsPane:
				if (m_detailsPane) m_detailsPane->setVisible(enabled);
				break;
			default:
				break; // RowContextMenu handled by concrete views
			}
		}
		bool QAbstractLogWidget::isFeatureEnabled(Feature f) const
		{
			if (f < 0 || f >= __featureCount)
				return false;
			return m_features[f];
		}
		void QAbstractLogWidget::refreshMatchCount()
		{
			if (!m_matchCountLabel)
				return;
			m_matchCountLabel->setText(QString::number(matchCount()));
		}
		void QAbstractLogWidget::soloContext(LoggerID id)
		{
			for (auto& kv : m_contextData)
			{
				if (!kv.second.checkBox) continue;
				kv.second.checkBox->setChecked(kv.first == id);
			}
		}
		void QAbstractLogWidget::hideContext(LoggerID id)
		{
			auto it = m_contextData.find(id);
			if (it != m_contextData.end() && it->second.checkBox)
				it->second.checkBox->setChecked(false);
		}
		void QAbstractLogWidget::setContextEnabled(LoggerID id, bool enabled)
		{
			auto it = m_contextData.find(id);
			if (it != m_contextData.end() && it->second.checkBox)
				it->second.checkBox->setChecked(enabled);
		}
		void QAbstractLogWidget::setSearchTextProgrammatic(const QString& text, bool regex)
		{
			if (m_searchRegexCheckBox)
				m_searchRegexCheckBox->setChecked(regex);
			if (m_searchLineEdit)
				m_searchLineEdit->setText(text);
		}

		void QAbstractLogWidget::onSearchLineEditChanged(const QString&)
		{
			emitSearchFilter();
		}
		void QAbstractLogWidget::onSearchRegexToggled(int)
		{
			emitSearchFilter();
		}
		void QAbstractLogWidget::emitSearchFilter()
		{
			if (!m_searchLineEdit || !m_searchRegexCheckBox)
				return;
			onSearchTextChanged(m_searchLineEdit->text(), m_searchRegexCheckBox->isChecked());
		}
		void QAbstractLogWidget::onSearchTextChanged(const QString& text, bool regex)
		{
			LOGGER_UNUSED(text);
			LOGGER_UNUSED(regex);
		}
	

		void QAbstractLogWidget::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			LOGGER_UNUSED(index);
			LOGGER_UNUSED(level);
			LOGGER_UNUSED(isChecked);
		}
		void QAbstractLogWidget::onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text)
		{
			LOGGER_UNUSED(lineEdit);
			if (index > 0)
				return;
			for (auto& loggerData : m_contextData)
			{
				if (loggerData.second.checkBox->text().contains(text.c_str(), Qt::CaseInsensitive))
				{
					loggerData.second.checkBox->setVisible(true);
				}
				else
				{
					loggerData.second.checkBox->setVisible(false);
				}
			}
		}
		void QAbstractLogWidget::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
		{
			LOGGER_UNUSED(context);
			LOGGER_UNUSED(isChecked);
			int checkedCount = 0;
			for (auto& loggerData : m_contextData)
				checkedCount += loggerData.second.checkBox->isChecked();
			m_ignoreAllContextCheckBox_signals = true;
			if (checkedCount == m_contextData.size())
				ui->allContext_checkBox->setCheckState(Qt::Checked);
			else if (checkedCount > 0)
				ui->allContext_checkBox->setCheckState(Qt::PartiallyChecked);
			else
				ui->allContext_checkBox->setCheckState(Qt::Unchecked);
			m_ignoreAllContextCheckBox_signals = false;
		}
	}
}
#endif