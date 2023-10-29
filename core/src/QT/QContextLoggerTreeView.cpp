#include "QT/QContextLoggerTreeView.h"

#ifdef LOGGER_QT
#include "ui_QContextLoggerTreeView.h"
#include <QTreeWidget>
namespace Log
{

	QContextLoggerTreeView::QContextLoggerTreeView(QWidget* parent)
		: QWidget(parent)
		, ui(new Ui::QContextLoggerTreeView)
	{
		ui->setupUi(this);



	}
	QContextLoggerTreeView::~QContextLoggerTreeView()
	{

	}
	void QContextLoggerTreeView::updateView()
	{
		//ui->treeView->model()->
	}
}
#endif