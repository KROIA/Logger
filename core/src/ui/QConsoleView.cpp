#include "ui/QConsoleView.h"

#ifdef LOGGER_QT
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
			m_consoleWidget = new QConsoleWidget(this);

			setContentWidget(m_consoleWidget);
		}
		QConsoleView::~QConsoleView()
		{
			/*auto loggers = getAttachedLoggers();
			for (auto& logger : loggers)
			{
				m_consoleWidget->detachLogger(*logger);
			}*/
		}

		void QConsoleView::setDateTimeFormat(DateTime::Format format)
		{
			m_consoleWidget->setDateTimeFormat(format);
		}
		DateTime::Format QConsoleView::getDateTimeFormat() const
		{
			return m_consoleWidget->getDateTimeFormat();
		}
		void QConsoleView::on_clear_pushButton_clicked()
		{
			QAbstractLogView::on_clear_pushButton_clicked();
			m_consoleWidget->clear();
		}
		/*void QConsoleView::onAllContextCheckBoxStateChanged(int state)
		{
			QAbstractLogView::onAllContextCheckBoxStateChanged(state);
		}
		
		void QConsoleView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onNewSubscribed(logger);
			//m_consoleWidget->attachLogger(logger);
		}
		void QConsoleView::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onUnsubscribed(logger);
			//m_consoleWidget->detachLogger(logger);
		}*/

		void QConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			QAbstractLogView::onLevelCheckBoxChanged(index, level, isChecked);
			m_consoleWidget->setLevelVisibility(level, isChecked);
		}
		void QConsoleView::onContextCheckBoxChanged(ContextData* context, bool isChecked)
		{
			QAbstractLogView::onContextCheckBoxChanged(context, isChecked);
			m_consoleWidget->setContextVisibility(context->loggerInfo->id, isChecked);
		}

		void QConsoleView::onNewMessage(const Message& m)
		{
			m_consoleWidget->onNewMessage(m);
		}

	}
}
#endif