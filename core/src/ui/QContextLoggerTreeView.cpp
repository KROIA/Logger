#include "ui/QContextLoggerTreeView.h"

#ifdef LOGGER_QT
#include "ui_QAbstractLogView.h"
#include <QTreeWidget>
#include <QMetaType>


namespace Log
{
	namespace UI
	{

		QContextLoggerTreeView::QContextLoggerTreeView(QWidget* parent)
			: QAbstractLogView(parent)
		{
			m_treeWidget = new QTreeWidget();
			setContentWidget(m_treeWidget);
			m_treeItem = new Receiver::QContextLoggerTree(m_treeWidget);
		}
		QContextLoggerTreeView::~QContextLoggerTreeView()
		{

		}

		void QContextLoggerTreeView::setDateTimeFormat(DateTime::Format format)
		{
			m_treeItem->setDateTimeFormat(format);
		}
		DateTime::Format QContextLoggerTreeView::getDateTimeFormat() const
		{
			return m_treeItem->getDateTimeFormat();
		}
		void QContextLoggerTreeView::on_clear_pushButton_clicked()
		{
			QAbstractLogView::on_clear_pushButton_clicked();
			m_treeItem->clearMessages();
		}
		/*void QContextLoggerTreeView::onAllContextCheckBoxStateChanged(int state)
		{
			QAbstractLogView::onAllContextCheckBoxStateChanged(state);
		}
		
		void QContextLoggerTreeView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onNewSubscribed(logger);
		}
		void QContextLoggerTreeView::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onUnsubscribed(logger);
		}*/

		/*void QContextLoggerTreeView::onContextCreate(Logger::ContextLogger& logger)
		{
			QAbstractLogView::onContextCreate(logger);
			//addContext(logger);
		}*/
		/*void QContextLoggerTreeView::onContextDestroy(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onContextDestroy(logger);
			//m_treeItem->loggerDeleted(logger.getID());
		}*/
		void QContextLoggerTreeView::addContext(Logger::AbstractLogger& logger)
		{
			m_treeItem->addContext(logger);
			QAbstractLogView::addContext(logger);
		}


		void QContextLoggerTreeView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			QAbstractLogView::onLevelCheckBoxChanged(index, level, isChecked);
			m_treeItem->setLevelVisibility(level, isChecked);
		}
		void QContextLoggerTreeView::onContextCheckBoxChanged(ContextData* context, bool isChecked)
		{
			QAbstractLogView::onContextCheckBoxChanged(context, isChecked);
			m_treeItem->setContextVisibility(context->loggerInfo->id, isChecked);
		}

		void QContextLoggerTreeView::onNewMessage(const Message& m)
		{
			m_treeItem->onNewMessage(m);
		}
	}
}
#endif