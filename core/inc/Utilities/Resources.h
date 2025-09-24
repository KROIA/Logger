#pragma once

#include "Logger_base.h"


#ifdef QT_WIDGETS_LIB
#include <unordered_map>
#include <string>
#include <QIcon>


namespace Log
{
	class LOGGER_API Resources
	{
	public:
		static const QIcon& getIcon(const std::string& name);
		static const QString& getStylesheet(const std::string& name);
		
		static const QIcon& getIconTrace();
		static const QIcon& getIconDebug();
		static const QIcon& getIconInfo();
		static const QIcon& getIconWarning();
		static const QIcon& getIconError();
		static const QIcon& getIconSearch();
		static const QIcon& getIconReload();

		static const QString getDarkStylesheet();
	private:
		Resources();
		void initResources();
		void loadIcons();
		void loadStylesheets();
		void loadStylesheet(const QString& name);
		static Resources& instance();

		std::unordered_map<std::string, QIcon> m_icons;
		std::unordered_map<std::string, QString> m_stylesheets;

		const QString m_imageFilePath;
		const QString m_imageFileEnding;

		const QString m_styleFilePath;
		const QString m_styleFileEnding;
	};
}
#endif