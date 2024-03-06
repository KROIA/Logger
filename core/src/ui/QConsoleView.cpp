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

		}

		void QConsoleView::setDateTimeFormat(DateTime::Format format)
		{
			m_consoleWidget->setDateTimeFormat(format);
		}
		DateTime::Format QConsoleView::getDateTimeFormat() const
		{
			return m_consoleWidget->getDateTimeFormat();
		}
		void QConsoleView::getSaveVisibleMessages(std::vector<Logger::AbstractLogger::LoggerSnapshotData>& list) const
		{
			std::vector< Logger::AbstractLogger::LoggerMetaInfo> contexts = getContexts();
			std::unordered_map<Logger::AbstractLogger::LoggerID, Logger::AbstractLogger::LoggerSnapshotData*> contextMap;
			for (auto& context : contexts)
			{
				Logger::AbstractLogger::LoggerSnapshotData data(context);
				list.push_back(data);
			}
			for (auto& snapshot : list)
			{
				contextMap[snapshot.metaInfo.id] = &snapshot;
			}
			std::vector<Log::Message::SnapshotData> messages;
			m_consoleWidget->getSaveVisibleMessages(messages);

			for (auto& message : messages)
			{
				auto it = contextMap.find(message.loggerID);
				if (it != contextMap.end())
				{
					it->second->messages.push_back(message);
				}
			}
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
		void QConsoleView::onContextCheckBoxChanged(ContextData const* context, bool isChecked)
		{
			QAbstractLogView::onContextCheckBoxChanged(context, isChecked);
			m_consoleWidget->setContextVisibility(context->loggerInfo->id, isChecked);
		}

		void QConsoleView::onDateTimeFilterChanged(const DateTimeFilter& filter)
		{
			m_consoleWidget->setDateTimeFilter(filter);
		}

		void QConsoleView::onNewMessage(const Message& m)
		{
			m_consoleWidget->onNewMessage(m);
		}

	}
}
#endif