#include "QT/QContextLoggerTreeView.h"

#ifdef LOGGER_QT
#include "ui_QContextLoggerTreeView.h"
#include <QTreeWidget>
#include <QMetaType>

namespace Log
{
	class STATIC_RESOURCE_LOADER
	{
			STATIC_RESOURCE_LOADER()
			{
				// name of the resource file = "icons.qrc"
				Q_INIT_RESOURCE(icons);
			}
		static STATIC_RESOURCE_LOADER s_loader;
	};
	STATIC_RESOURCE_LOADER STATIC_RESOURCE_LOADER::s_loader;



	QContextLoggerTreeView::QContextLoggerTreeView(QWidget* parent)
		: QWidget(parent)
		, ui(new Ui::QContextLoggerTreeView)
	{
		ui->setupUi(this);
		m_treeWidget = new QTreeWidget();
		
		ui->tree_widget->layout()->addWidget(m_treeWidget);

		m_treeItem = new QContextLoggerTree(m_treeWidget);


		m_updateTimer.setInterval(100);
		connect(&m_updateTimer, &QTimer::timeout, this, &QContextLoggerTreeView::onUpdateTimer);
		m_updateTimer.start();

		//logger.subscribeToUI(m_treeItem);
		//updateView();
		

		
	}
	QContextLoggerTreeView::~QContextLoggerTreeView()
	{

	}
	void QContextLoggerTreeView::connectLogger(ContextLogger& logger)
	{
		m_treeItem->attachLogger(logger);
	}
	void QContextLoggerTreeView::disconnectLogger(ContextLogger& logger)
	{
		m_treeItem->detachLogger(logger);
	}

	void QContextLoggerTreeView::onUpdateTimer()
	{
		
	}

	//void QContextLoggerTreeView::updateView()
	//{
		//ui->treeView->model()->
		/*for (Log::ContextLogger* l : Log::ContextLogger::getAllLogger())
		{
			Log::QContextLoggerTree* item = new Log::QContextLoggerTree(m_treeWidget);
			item->updateData(*l);
			//widget->addTopLevelItem(l->getTreeWidgetItem());
		}*/
		//m_treeWidget->clear();
		
		//m_treeItem->updateData(m_logger);
	//}
}
#endif