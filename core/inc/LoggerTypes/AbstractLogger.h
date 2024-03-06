#pragma once

#include "Logger_base.h"
#include "LogMessage.h"
#include "LoggerInterface.h"
#include "Utilities/Signal.h"
#include "LogColor.h"
#include <memory>

#ifdef LOGGER_QT
#include <QIcon>
#endif

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
			void log(Level level, const std::string& msg) override;
			void log(Level level, const Color& col, const std::string& msg) override;

			void log(const char* msg) override;
			void log(Level level, const char* msg) override;
			void log(Level level, const Color& col, const char* msg) override;

			void setTabCount(unsigned int tabCount) override;
			void tabIn() override;
			void tabOut() override;
			unsigned int getTabCount() const override;

#ifdef LOGGER_QT
			void setIcon(const QIcon& icon);
			const QIcon& getIcon() const;
#endif

			virtual void clear();

			const DateTime& getCreationDateTime() const;
			const std::vector<Message>& getMessages() const;

			LoggerID getID() const;

			struct LoggerMetaInfo
			{
				LoggerID id;
				LoggerID parentId;
				std::string name;
				DateTime creationTime;
				Color color;
				unsigned int tabCount;
				bool enabled;
				bool isAlive;

				LoggerMetaInfo(LoggerID id,
					LoggerID parentID,
					const std::string &name,
					const DateTime &creationTime,
					const Color &color,
					unsigned int tabCount,
					bool enabled,
					bool isAlive) 
					: id(id)
					, parentId(parentID)
					, name(name)
					, creationTime(creationTime)
					, color(color)
					, tabCount(tabCount)
					, enabled(enabled)
					, isAlive(isAlive) 
				{}
				LoggerMetaInfo(const LoggerMetaInfo &other) 
					: id(other.id)
					, parentId(other.parentId)
					, name(other.name)
					, creationTime(other.creationTime)
					, color(other.color)
					, tabCount(other.tabCount)
					, enabled(other.enabled)
					, isAlive(other.isAlive) 
				{}
			};
			struct LoggerSnapshotData
			{
				LoggerMetaInfo metaInfo;
				std::vector<Message::SnapshotData> messages;
				LoggerSnapshotData(const LoggerMetaInfo& info)
					: metaInfo(info)
				{	}
				LoggerSnapshotData(const LoggerMetaInfo& info,
								   const std::vector<Message::SnapshotData>& msgs)
					: metaInfo(info)
					, messages(msgs)
				{	}
			};
			std::shared_ptr<const LoggerMetaInfo> getMetaInfo() const;

			// Signals
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onNewMessage, const Message&);
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onClear, AbstractLogger&);
			DECLARE_SIGNAL_CONNECT_DISCONNECT(onDelete, AbstractLogger&);

		protected:
			virtual void logInternal(const Message& msg);

			void emitNewMessage(const Message& msg);

			LoggerMetaInfo m_metaInfo;
			std::shared_ptr<LoggerMetaInfo> m_sharedMetaInfo;

			std::vector<Message> m_messages;

#ifdef LOGGER_QT
			QIcon m_icon;
#endif
		private:
			
			


			Signal<const Message&> onNewMessage;
			Signal<AbstractLogger&> onClear;
			Signal<AbstractLogger&> onDelete;

			static LoggerID s_idCounter;
		};
	}
}