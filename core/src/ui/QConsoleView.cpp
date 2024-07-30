#include "ui/QConsoleView.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogView.h"
#include <QTreeWidget>
#include <QMetaType>


namespace Log
{
	namespace UI
	{

		QConsoleView::QConsoleView(QWidget* parent)
			: QAbstractLogView(parent)
		{
			setWindowTitle("Console");
			m_consoleWidget = new QConsoleWidget(this);
			setContentWidget(m_consoleWidget);
			postConstructorInit();
		}
		QConsoleView::~QConsoleView()
		{

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
			QAbstractLogView::on_clear_pushButton_clicked();
			m_consoleWidget->clear();
		}

		void QConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			QAbstractLogView::onLevelCheckBoxChanged(index, level, isChecked);
			m_consoleWidget->setLevelVisibility(level, isChecked);
		}
		void QConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
		{
			QAbstractLogView::onContextCheckBoxChanged(context, isChecked);
			m_consoleWidget->setContextVisibility(context.id, isChecked);
		}

		void QConsoleView::onDateTimeFilterChanged(const DateTimeFilter& filter)
		{
			m_consoleWidget->setDateTimeFilter(filter);
		}

		void QConsoleView::onNewLogger(LogObject::Info loggerInfo)
		{
			QAbstractLogView::onNewLogger(loggerInfo);
		}
		void QConsoleView::onLoggerInfoChanged(LogObject::Info info)
		{
			QAbstractLogView::onLoggerInfoChanged(info);
		}
		void QConsoleView::onLogMessage(Message message)
		{
			QAbstractLogView::onLogMessage(message);
			m_consoleWidget->onNewMessage(message);
		}
		void QConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			QAbstractLogView::onChangeParent(childID, newParentID);
		}

	}
}
#endif