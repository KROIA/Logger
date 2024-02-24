#pragma once

#include "Logger_base.h"


#ifdef LOGGER_QT
#include <unordered_map>
#include <string>
#include <QIcon>


namespace Log
{
	class LOGGER_EXPORT Resources
	{
	public:
		static const QIcon& getIcon(const std::string& name);
		
		static const QIcon& getIconTrace();
		static const QIcon& getIconDebug();
		static const QIcon& getIconInfo();
		static const QIcon& getIconWarning();
		static const QIcon& getIconError();
	private:
		Resources();
		void initResources();
		void loadIcons();
		static Resources& instance();

		std::unordered_map<std::string, QIcon> m_icons;
	};
}
#endif