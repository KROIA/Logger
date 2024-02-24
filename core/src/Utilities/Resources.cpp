#include "Utilities/Resources.h"
#include <vector>

namespace Log
{

#ifdef LOGGER_QT
	const QIcon& Resources::getIcon(const std::string& name)
	{
		return instance().m_icons[name];
	}

	const QIcon& Resources::getIconTrace()
	{
		return getIcon("trace");
	}
	const QIcon& Resources::getIconDebug()
	{
		return getIcon("debug");
	}
	const QIcon& Resources::getIconInfo()
	{
		return getIcon("info");
	}
	const QIcon& Resources::getIconWarning()
	{
		return getIcon("warning");
	}
	const QIcon& Resources::getIconError()
	{
		return getIcon("error");
	}


	Resources::Resources()
	{
		initResources();
		loadIcons();
	}
	void Resources::initResources()
	{
		// name of the resource file = "icons.qrc"
		Q_INIT_RESOURCE(icons);
	}
	void Resources::loadIcons()
	{
		m_icons.clear();
		const std::string fileEnding = ".png";
		const std::string path = ":/icons/";

		// List of icon names
		const std::vector<std::string> iconNames = 
		{ 
			"trace", 
			"debug", 
			"info", 
			"warning", 
			"error" 
		};
		for(const auto& name : iconNames)
		{
			m_icons[name] = QIcon(QString::fromStdString(path + name + fileEnding));
		}
	}
	Resources& Resources::instance()
	{
		static Resources instance;
		return instance;
	}
#endif
}