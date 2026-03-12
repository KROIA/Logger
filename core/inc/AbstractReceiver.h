#pragma once

#include "Logger_base.h"
#include "LogManager.h"

#include <QObject>
#include <array>

namespace Log
{
	class AbstractReceiver;
	class MessageFilter;
	namespace Internal
	{
		// Encapsulating signal handling in a private class
		class LOGGER_API SignalReceiver : public QObject
		{
			friend class AbstractReceiver;
			Q_OBJECT
			public:
			SignalReceiver(AbstractReceiver* receiver);
			~SignalReceiver() {};

			void setLevelFilter(Level level, bool enable);

			public slots:
			void onNewLogger(LogObject::Info loggerInfo);
			void onLoggerInfoChanged(LogObject::Info info);
			void onLogMessage(Message message);
			void onChangeParent(LoggerID childID, LoggerID newParentID);

			private:
			AbstractReceiver* receiver;
			std::shared_ptr<MessageFilter> m_messageFilter = nullptr;
			std::array<bool, Level::__count> m_levelFilter;
		};
	}

	// Reimplement this class to create a new receiver type.
	class LOGGER_API AbstractReceiver
	{
		friend class Internal::SignalReceiver;
	public:
		AbstractReceiver();
		virtual ~AbstractReceiver();

		void setLevelFilter(Level level, bool enable);

		template<typename T, typename... Args>
		std::shared_ptr<T> setFlter(Args&&... args)
		{
			std::shared_ptr<T> filter = std::make_shared<T>(std::forward<Args>(args)...);
			signalReceiver.m_messageFilter = filter;
			return filter;
		}
		void setFilter(std::shared_ptr<MessageFilter> filter) 
		{ 
			signalReceiver.m_messageFilter = filter; 
		}

		void clearFilter();
		std::shared_ptr<MessageFilter> getFilter() const { return signalReceiver.m_messageFilter; }
	protected:
		virtual void onNewLogger(LogObject::Info loggerInfo) = 0;
		virtual void onLoggerInfoChanged(LogObject::Info info) = 0;
		virtual void onLogMessage(Message message) = 0;
		virtual void onChangeParent(LoggerID childID, LoggerID newParentID) = 0;
	private:
		Internal::SignalReceiver signalReceiver;
	};

	
}