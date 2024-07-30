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
			QAbstractLogWidget::on_clear_pushButton_clicked();
			m_consoleWidget->clear();
		}

		void QConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
			m_consoleWidget->setLevelVisibility(level, isChecked);
		}
		void QConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
		{
			QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
			m_consoleWidget->setContextVisibility(context.id, isChecked);
		}

		void QConsoleView::onDateTimeFilterChanged(const DateTimeFilter& filter)
		{
			m_consoleWidget->setDateTimeFilter(filter);
		}

		void QConsoleView::onNewLogger(LogObject::Info loggerInfo)
		{
			QAbstractLogWidget::onNewLogger(loggerInfo);
		}
		void QConsoleView::onLoggerInfoChanged(LogObject::Info info)
		{
			QAbstractLogWidget::onLoggerInfoChanged(info);
		}
		void QConsoleView::onLogMessage(Message message)
		{
			QAbstractLogWidget::onLogMessage(message);
			m_consoleWidget->onNewMessage(message);
		}
		void QConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			QAbstractLogWidget::onChangeParent(childID, newParentID);
		}

	}
}
#endif