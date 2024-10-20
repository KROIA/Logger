#include "ui/QTreeConsoleView.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogWidget.h"
#include <QTreeWidget>
#include <QMetaType>


namespace Log
{
	namespace UI
	{

		QTreeConsoleView::QTreeConsoleView(QWidget* parent)
			: QAbstractLogWidget(parent)
		{
			setWindowTitle("Console tree view");
			m_treeWidget = new QTreeWidget();
			setContentWidget(m_treeWidget);
			m_treeItem = new UIWidgets::QContextLoggerTreeWidget(m_treeWidget);

			connect(this, &QTreeConsoleView::messageQueued, this, &QTreeConsoleView::onMessageQueued, Qt::QueuedConnection);
			postConstructorInit();
		}
		QTreeConsoleView::~QTreeConsoleView()
		{

		}

		void QTreeConsoleView::createStaticInstance()
		{
			QTreeConsoleView*& instancePtr = getStaticInstance();
			if (instancePtr)
				return;
			instancePtr = new QTreeConsoleView();
		}
		void QTreeConsoleView::destroyStaticInstance()
		{
			QTreeConsoleView*& instancePtr = getStaticInstance();
			if (instancePtr)
			{
				delete instancePtr;
				instancePtr = nullptr;
			}
		}
		QTreeConsoleView*& QTreeConsoleView::getStaticInstance()
		{
			static QTreeConsoleView* instancePtr = nullptr;
			return instancePtr;
		}

		void QTreeConsoleView::setDateTimeFormat(DateTime::Format format)
		{
			m_treeItem->setDateTimeFormat(format);
		}
		DateTime::Format QTreeConsoleView::getDateTimeFormat() const
		{
			return m_treeItem->getDateTimeFormat();
		}
		void QTreeConsoleView::getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const
		{
			QMutexLocker locker(&m_mutex);
			m_treeItem->getSaveVisibleMessages(list);
		}
		void QTreeConsoleView::on_clear_pushButton_clicked()
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QMutexLocker locker(&m_mutex);
			QAbstractLogWidget::on_clear_pushButton_clicked();
			m_treeItem->clearMessages();
		}


		void QTreeConsoleView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onLevelCheckBoxChanged(index, level, isChecked);
			m_treeItem->setLevelVisibility(level, isChecked);
		}
		void QTreeConsoleView::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onContextCheckBoxChanged(context, isChecked);
			m_treeItem->setContextVisibility(context.id, isChecked);
		}
		void QTreeConsoleView::onDateTimeFilterChanged(const DateTimeFilter& filter)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			m_treeItem->setDateTimeFilter(filter);
		}

		void QTreeConsoleView::onNewLogger(LogObject::Info loggerInfo)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onNewLogger(loggerInfo);
			m_treeItem->addContext(loggerInfo);
		}
		void QTreeConsoleView::onLogMessage(Message message)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			QAbstractLogWidget::onLogMessage(message);
			m_treeItem->onNewMessage(message);
		}
		void QTreeConsoleView::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			m_treeItem->setParent(childID, newParentID);
		}
		void QTreeConsoleView::onMessageQueued(QPrivateSignal*)
		{
			LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
			std::vector<Message> cpy;
			{
				QMutexLocker locker(&m_mutex);
				cpy = m_messageQueue;
				m_messageQueue.clear();
			}
			QMutexLocker locker(&m_mutex);
			for (auto& m : cpy)
				m_treeItem->onNewMessage(m);
		}
	}
}
#endif