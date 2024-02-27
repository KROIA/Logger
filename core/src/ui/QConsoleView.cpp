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
			auto loggers = getAttachedLoggers();
			for (auto& logger : loggers)
			{
				m_consoleWidget->detachLogger(*logger);
			}
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
			m_consoleWidget->clear();
		}
		void QConsoleView::onAllContextCheckBoxStateChanged(int state)
		{
			QAbstractLogView::onAllContextCheckBoxStateChanged(state);
		}
		
		void QConsoleView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onNewSubscribed(logger);
			m_consoleWidget->attachLogger(logger);
		}
		void QConsoleView::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onUnsubscribed(logger);
			m_consoleWidget->detachLogger(logger);
		}

		/*void QConsoleView::onContextCreate(Logger::ContextLogger& logger)
		{
			QAbstractLogView::onContextCreate(logger);	
		}*/
		/*void QConsoleView::onContextDestroy(Logger::ContextLogger& logger)
		{
			QAbstractLogView::onContextDestroy(logger);
		}*/


		void QConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			QAbstractLogView::onLevelCheckBoxChanged(index, level, isChecked);
			m_consoleWidget->setLevelVisibility(level, isChecked);
		}
		/*void QConsoleView::onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text)
		{
			QAbstractLogView::onFilterTextChanged(index, lineEdit, text);
		}*/
		void QConsoleView::onContextCheckBoxChanged(ContextData* context, bool isChecked)
		{
			QAbstractLogView::onContextCheckBoxChanged(context, isChecked);
			m_consoleWidget->setContextVisibility(*context->logger, isChecked);
		}
		/*void QConsoleView::onNewContextCheckBoxCreated(ContextData* context)
		{
			QAbstractLogView::onNewContextCheckBoxCreated(context);
		}*/
		/*void QConsoleView::onContextCheckBoxDestroyed(ContextData* context)
		{
			QAbstractLogView::onContextCheckBoxDestroyed(context);
		}*/
	}
}
#endif