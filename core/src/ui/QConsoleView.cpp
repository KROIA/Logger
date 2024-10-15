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

		void QConsoleView::on_clear_pushButton_clicked()
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::on_clear_pushButton_clicked();
			m_consoleWidget->clear();
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

		void QConsoleView::onNewLogger(LogObject::Info loggerInfo)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onNewLogger(loggerInfo);
		}
		void QConsoleView::onLoggerInfoChanged(LogObject::Info info)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onLoggerInfoChanged(info);
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