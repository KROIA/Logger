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
			//m_treeWidget = new QTreeWidget();

			m_treeView = new QTreeView();
			m_treeModel = new QTreeModel(this);
			m_treeView->setModel(m_treeModel);

			setContentWidget(m_treeView);
			//setContentWidget(m_treeWidget);
			
			//m_treeItem = new Receiver::QContextLoggerTree(m_treeWidget);
		}
		QContextLoggerTreeView::~QContextLoggerTreeView()
		{
			/*auto loggers = getAttachedLoggers();
			for (auto& logger : loggers)
			{
				m_treeItem->detachLogger(*logger);
			}*/
			delete m_treeView;
			delete m_treeModel;
		}

		void QContextLoggerTreeView::setDateTimeFormat(DateTime::Format format)
		{
			//m_treeItem->setDateTimeFormat(format);
		}
		DateTime::Format QContextLoggerTreeView::getDateTimeFormat() const
		{
			//return m_treeItem->getDateTimeFormat();
			return DateTime::Format::hourMinuteSecondMillisecond;
		}
		void QContextLoggerTreeView::on_clear_pushButton_clicked()
		{
			auto loggers = getAttachedLoggers();
			for (auto& logger : loggers)
			{
				logger->clear();
			}
		}
		void QContextLoggerTreeView::onAllContextCheckBoxStateChanged(int state)
		{
			QAbstractLogView::onAllContextCheckBoxStateChanged(state);
		}
		
		void QContextLoggerTreeView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onNewSubscribed(logger);
			Logger::ContextLogger * contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
			if(contextLogger)
				m_treeModel->attachLoggerAndChilds(*contextLogger);
			else
				m_treeModel->attachLogger(logger);
			//m_treeItem->attachLogger(logger);
		}
		void QContextLoggerTreeView::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractLogView::onUnsubscribed(logger);
			Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);
			if (contextLogger)
				m_treeModel->detachLoggerAndChilds(*contextLogger);
			else
				m_treeModel->detachLogger(logger);
			//m_treeItem->detachLogger(logger);
		}

		/*void QContextLoggerTreeView::onContextCreate(Logger::ContextLogger& logger)
		{
			QAbstractLogView::onContextCreate(logger);	
		}*/
		/*void QContextLoggerTreeView::onContextDestroy(Logger::ContextLogger& logger)
		{
			QAbstractLogView::onContextDestroy(logger);
		}*/


		void QContextLoggerTreeView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			QAbstractLogView::onLevelCheckBoxChanged(index, level, isChecked);
			//m_treeItem->setLevelVisibility(level, isChecked);
		}
		/*void QContextLoggerTreeView::onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text)
		{
			QAbstractLogView::onFilterTextChanged(index, lineEdit, text);
		}*/
		void QContextLoggerTreeView::onContextCheckBoxChanged(ContextData* context, bool isChecked)
		{
			QAbstractLogView::onContextCheckBoxChanged(context, isChecked);
			Logger::ContextLogger *contextLogger = dynamic_cast<Logger::ContextLogger*>(context->logger);
			//if(contextLogger)
			//	m_treeItem->setContextVisibility(*contextLogger, isChecked);
		}
		/*void QContextLoggerTreeView::onNewContextCheckBoxCreated(ContextData* context)
		{
			QAbstractLogView::onNewContextCheckBoxCreated(context);
		}*/
		/*void QContextLoggerTreeView::onContextCheckBoxDestroyed(ContextData* context)
		{
			QAbstractLogView::onContextCheckBoxDestroyed(context);
		}*/
	}
}
#endif