#include "Utilities/Resources.h"
#include <vector>
#ifdef QT_WIDGETS_LIB
#include <QDebug>
#include <QFile>
#endif
namespace Log
{

#ifdef QT_WIDGETS_LIB

	const QIcon& Resources::getIcon(const std::string& name)
	{
		Resources &instance = Resources::instance(); 
		const auto it = instance.m_icons.find(name);
		if(it != instance.m_icons.end())
			return it->second;
		qDebug() << __PRETTY_FUNCTION__ << "Icon not in initial list: " << name.c_str();
		instance.m_icons[name] = QIcon(instance.m_imageFilePath + QString::fromStdString(name) + instance.m_imageFileEnding);
		return instance.m_icons[name];
	}
	const QString& Resources::getStylesheet(const std::string& name)
	{
		Resources &instance = Resources::instance();
		const auto it = instance.m_stylesheets.find(name);
		if (it != instance.m_stylesheets.end())
			return it->second;
		qDebug() << __PRETTY_FUNCTION__ << "Stylesheet not in initial list: " << name.c_str();
		instance.loadStylesheet(QString::fromStdString(name));
		const auto it2 = instance.m_stylesheets.find(name);
		if (it2 != instance.m_stylesheets.end())
			return it2->second;
		static QString empty;
		return empty;
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
	const QIcon& Resources::getIconReload()
	{
		return getIcon("reload");
	}

	const QString Resources::getDarkStylesheet()
	{
		return getStylesheet("darkstyle");
	}


	Resources::Resources()
		: m_imageFilePath(":/icons/")
		, m_imageFileEnding(".png")
		, m_styleFilePath(":/styles/")
		, m_styleFileEnding(".qss")
	{
		initResources();
		loadIcons();
		loadStylesheets();
	}
	void Resources::initResources()
	{
		// name of the resource file = "icons.qrc"
		Q_INIT_RESOURCE(icons);
		Q_INIT_RESOURCE(darkstyle);
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
			"magnifying-glass",
			"reload"
		};
		for(const auto& name : iconNames)
		{
			m_icons[name] = QIcon(m_imageFilePath + QString::fromStdString(name) + m_imageFileEnding);
		}
	}
	void Resources::loadStylesheets()
	{
		m_stylesheets.clear();
		// List of icon names
		const std::vector<QString> styles =
		{
			"darkstyle",
		};
		for (const auto& style : styles)
		{
			loadStylesheet(style);
		}
	}
	void Resources::loadStylesheet(const QString& name)
	{
		QString stylePath = m_styleFilePath + name+"/"+name + m_styleFileEnding;
		QFile styleFile(stylePath); // Load stylesheet from resources
		if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream stream(&styleFile);
			m_stylesheets[name.toStdString()] = stream.readAll();
			styleFile.close();
		}
		else {
			qDebug() << "Failed to load stylesheet: " << name;
		}
	}
	Resources& Resources::instance()
	{
		static Resources instance;
		return instance;
	}
#endif
}