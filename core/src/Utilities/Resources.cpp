#include "Utilities/Resources.h"
#include <vector>
#ifdef LOGGER_QT
#include <QDebug>
#endif
namespace Log
{

#ifdef LOGGER_QT

	const QIcon& Resources::getIcon(const std::string& name)
	{
		Resources &instance = Resources::instance(); 
		const auto it = instance.m_icons.find(name);
		if(it != instance.m_icons.end())
			return it->second;
		qDebug() << __PRETTY_FUNCTION__ << "Icon not in initial list: " << name.c_str();
		instance.m_icons[name] = QIcon(QString::fromStdString(instance.m_filePath + name + instance.m_fileEnding));
		return instance.m_icons[name];
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
	const QIcon& Resources::getIconSearch()
	{
		return getIcon("magnifying-glass");
	}


	Resources::Resources()
		: m_filePath(":/icons/")
		, m_fileEnding(".png")
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

		// List of icon names
		const std::vector<std::string> iconNames = 
		{ 
			"trace", 
			"debug", 
			"info", 
			"warning", 
			"error",
			"magnifying-glass"	
		};
		for(const auto& name : iconNames)
		{
			m_icons[name] = QIcon(QString::fromStdString(m_filePath + name + m_fileEnding));
		}
	}
	Resources& Resources::instance()
	{
		static Resources instance;
		return instance;
	}
#endif
}