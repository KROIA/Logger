#pragma once

#include "AbstractLogger.h"
#include <vector>
#include <sstream>


namespace Log
{
	class LOGGER_EXPORT ContextLogger : public AbstractLogger
	{
	public:
		

		ContextLogger(const std::string& name = "");
		ContextLogger(const ContextLogger& other);

		~ContextLogger();

		ContextLogger* createContext(const std::string& name);


		void clear();
		void destroyChilds();

		void toStringVector(std::vector<std::string>& list) const;
		friend std::ostream& operator<<(std::ostream& os, const ContextLogger& msg);


	private:
		void toStringVector(size_t depth, std::vector<std::string>& list) const;
		void logInternal(const Message& msg) override;

		DateTime m_creationsTime;
		std::vector<Message> m_messages;
		std::vector<ContextLogger*> m_childs;
		
	};
}