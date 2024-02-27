#pragma once

#include "Logger_base.h"
#include "LogMessage.h"
#include "LoggerInterface.h"
#include "Utilities/Signal.h"
#include "LogColor.h"

namespace Log
{
	namespace Logger
	{
		class LOGGER_EXPORT AbstractLogger : public LoggerInterface
		{
		public:
			typedef int LoggerID;

			explicit AbstractLogger(const std::string& name = "");
			AbstractLogger(const AbstractLogger& other);

			virtual ~AbstractLogger();

			void setEnabled(bool enable);
			bool isEnabled() const;

			void setName(const std::string& name);
			const std::string& getName() const;

			void setColor(const Color& col);
			const Color& getColor() const;

			void log(Message msg) override;

			void log(const std::string& msg) override;
			void log(const std::string& msg, Level level) override;
			void log(const std::string& msg, Level level, const Color& col) override;

			void log(const char* msg) override;
			void log(const char* msg, Level level) override;
			void log(const char* msg, Level level, const Color& col) override;

			void setTabCount(unsigned int tabCount) override;
			void tabIn() override;
			void tabOut() override;
			unsigned int getTabCount() const override;

			

			virtual void clear();

			const DateTime& getCreationDateTime() const;
			const std::vector<Message>& getMessages() const;

			LoggerID getID() const;

			// Signals
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onNewMessage, const Message&);
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onClear, AbstractLogger&);
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onDelete, AbstractLogger&);

		protected:
			virtual void logInternal(const Message& msg);

			void emitNewMessage(const Message& msg);
		private:
			bool m_enable;
			unsigned int m_tabCount;
			LoggerID m_id = 0;
			std::string m_name;
			DateTime m_creationsTime;
			Color m_color;
			std::vector<Message> m_messages;


			Signal<const Message&> onNewMessage;
			Signal<AbstractLogger&> onClear;
			Signal<AbstractLogger&> onDelete;

			static LoggerID s_idCounter;
		};
	}
}