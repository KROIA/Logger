#include "ui/QContextLoggerTreeView.h"

#ifdef QT_WIDGETS_LIB
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
			setWindowTitle("Console tree view");
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
		void QContextLoggerTreeView::getSaveVisibleMessages(std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list) const
		{
			QMutexLocker locker(&m_mutex);
			m_treeItem->getSaveVisibleMessages(list);
		}
		void QContextLoggerTreeView::on_clear_pushButton_clicked()
		{
			QMutexLocker locker(&m_mutex);
			QAbstractLogView::on_clear_pushButton_clicked();
			m_treeItem->clearMessages();
		}
		void QContextLoggerTreeView::addContext(Logger::AbstractLogger& logger)
		{
			QMutexLocker locker(&m_mutex);
			m_treeItem->addContext(logger);
			QAbstractLogView::addContext(logger);
		}


		void QContextLoggerTreeView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			QAbstractLogView::onLevelCheckBoxChanged(index, level, isChecked);
			m_treeItem->setLevelVisibility(level, isChecked);
		}
		void QContextLoggerTreeView::onContextCheckBoxChanged(ContextData const* context, bool isChecked)
		{
			QAbstractLogView::onContextCheckBoxChanged(context, isChecked);
			m_treeItem->setContextVisibility(context->loggerInfo->id, isChecked);
		}
		void QContextLoggerTreeView::onDateTimeFilterChanged(const DateTimeFilter& filter)
		{
			m_treeItem->setDateTimeFilter(filter);
		}

		void QContextLoggerTreeView::onNewMessage(const Message& m)
		{
			QMutexLocker locker(&m_mutex);
			m_treeItem->onNewMessage(m);
		}
	}
}
#endif