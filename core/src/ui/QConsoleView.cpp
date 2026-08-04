#include "ui/QConsoleView.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogWidget.h"
#include <QTreeWidget>
#include <QMetaType>


namespace Log
{
	namespace UI
	{

		QConsoleView::QConsoleView(QWidget* parent)
			: QAbstractLogWidget(parent)
		{
			setWindowTitle("Console");
			m_consoleWidget = new UIWidgets::QConsoleWidget(this);
			setContentWidget(m_consoleWidget);

			connect(m_consoleWidget, &UIWidgets::QConsoleWidget::filterChanged,
				this, &QConsoleView::refreshMatchCount);
			connect(m_consoleWidget, &UIWidgets::QConsoleWidget::requestSoloContext,
				this, [this](Log::LoggerID id) { soloContext(id); });
			connect(m_consoleWidget, &UIWidgets::QConsoleWidget::requestHideContext,
				this, [this](Log::LoggerID id) { hideContext(id); });
			connect(m_consoleWidget, &UIWidgets::QConsoleWidget::requestHideMessagesLike,
				this, [this](const QString& text) {
					setSearchTextProgrammatic(QStringLiteral("!") + text, false);
				});
			connect(m_consoleWidget, &UIWidgets::QConsoleWidget::selectionChangedMessage,
				this, [this](const Log::Message& msg, bool has) { updateDetailsFor(msg, has); });

			postConstructorInit();
		}
		QConsoleView::~QConsoleView()
		{

		}

		void QConsoleView::createStaticInstance()
		{
			QConsoleView*& instancePtr = getStaticInstance();
			if (instancePtr)
				return;
			instancePtr = new QConsoleView();
		}
		void QConsoleView::destroyStaticInstance()
		{
			QConsoleView*& instancePtr = getStaticInstance();
			if (instancePtr)
			{
				delete instancePtr;
				instancePtr = nullptr;
			}
		}
		QConsoleView*& QConsoleView::getStaticInstance()
		{
			static QConsoleView* instancePtr = nullptr;
			return instancePtr;
		}

		void QConsoleView::setFeatureEnabled(Feature f, bool enabled)
		{
			QAbstractLogWidget::setFeatureEnabled(f, enabled);
			if (f == RowContextMenu)
				m_consoleWidget->setContextMenuEnabled(enabled);
		}
		void QConsoleView::setDateTimeFormat(DateTime::Format format)
		{
			m_consoleWidget->setDateTimeFormat(format);
		}
		DateTime::Format QConsoleView::getDateTimeFormat() const
		{
			return m_consoleWidget->getDateTimeFormat();
		}
		void QConsoleView::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
		{
			m_consoleWidget->getSaveVisibleMessages(list);
		}
		void QConsoleView::clear()
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			m_consoleWidget->clear();
			QAbstractLogWidget::clear();
		}


		void QConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
			m_consoleWidget->setLevelVisibility(level, isChecked);
		}
		void QConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
			m_consoleWidget->setContextVisibility(context.id, isChecked);
		}

		void QConsoleView::onDateTimeFilterChanged(const DateTimeFilter& filter)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			m_consoleWidget->setDateTimeFilter(filter);
		}
		void QConsoleView::onSearchTextChanged(const QString& text, bool regex)
		{
			m_consoleWidget->setTextFilter(text, regex);
			refreshMatchCount();
		}
		int QConsoleView::matchCount() const
		{
			return m_consoleWidget->getMatchCount();
		}
		void QConsoleView::findNext(bool forward)
		{
			m_consoleWidget->findNext(forward);
		}

		void QConsoleView::onNewLogger(LogObject::Info loggerInfo)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onNewLogger(loggerInfo);
			m_consoleWidget->onNewLogger(loggerInfo);
		}
		void QConsoleView::onLoggerInfoChanged(LogObject::Info info)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onLoggerInfoChanged(info);
			m_consoleWidget->onNewLogger(info);
		}
		void QConsoleView::onLogMessage(Message message)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onLogMessage(message);
			m_consoleWidget->onNewMessage(message);
		}
		void QConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onChangeParent(childID, newParentID);
		}

	}
}
#endif