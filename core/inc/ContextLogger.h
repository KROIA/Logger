#pragma once

#include "AbstractLogger.h"
#include <vector>
#include <sstream>

namespace Log
{
#ifdef LOGGER_QT
	class QContextLoggerTreeWidgetItem;
#endif
	class LOGGER_EXPORT ContextLogger : public AbstractLogger
	{

		ContextLogger(const std::string& name, ContextLogger* parent);
	public:
		ContextLogger(const std::string& name = "");
		ContextLogger(const ContextLogger& other);

		~ContextLogger();

		static const std::vector<ContextLogger*>& getAllLogger();

		ContextLogger* createContext(const std::string& name);


		void clear();
		void destroyChilds();

		void toStringVector(std::vector<std::string>& list) const;
		friend std::ostream& operator<<(std::ostream& os, const ContextLogger& msg);

		const DateTime& getDateTime() const;
		const std::vector<Message>& getMessages() const;
		const std::vector<ContextLogger*>& getChilds() const;

#ifdef LOGGER_QT
		QContextLoggerTreeWidgetItem* getTreeWidgetItem();
#endif
	private:
		void toStringVector(size_t depth, std::vector<std::string>& list) const;
		void logInternal(const Message& msg) override;

#ifdef LOGGER_QT
		void updateTreeWidgetItem();
#endif

		DateTime m_creationsTime;
		std::vector<Message> m_messages;
		std::vector<ContextLogger*> m_childs;
		ContextLogger* m_parent;

#ifdef LOGGER_QT
		QContextLoggerTreeWidgetItem* m_treeWidget;
#endif
		
		static std::vector<ContextLogger*> s_allRootLoggers;
	};
}