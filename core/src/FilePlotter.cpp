#include "FilePlotter.h"
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include "Utilities/Export.h"

namespace Log
{
	FilePlotter::FilePlotter(const std::string& filePath, DateTime::Format format)
		: m_filePath(filePath)
		, m_dateTimeFormat(format)
	{
		// Open the file to stream all messages to
		QString filePathQString = QString::fromStdString(filePath);
		createDirectoryIfNotExists(filePathQString);
		m_file = new QFile(filePathQString);
		if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
		{
			// Error opening file
			delete m_file;
			m_file = nullptr;
		}
		else
		{
			// Write the file header
			QTextStream out(m_file);
			out << "[\n";
			out << QJsonDocument(Export::getFileHeader()).toJson();
			out << "]\n";
		}
	}
	FilePlotter::~FilePlotter()
	{
		if (m_file)
		{
			m_file->close();
			delete m_file;
		}
	}
	void FilePlotter::onNewLogger(LogObject::Info loggerInfo)
	{
		insertJson(loggerInfo.toJson());
	}
	void FilePlotter::onLoggerInfoChanged(LogObject::Info info)
	{
		insertJson(info.toJson());
	}
	void FilePlotter::onLogMessage(Message message)
	{
		insertJson(message.toJson());
	}
	void FilePlotter::onChangeParent(LoggerID childID, LoggerID newParentID)
	{
		QJsonObject obj;
		obj["childID"] = (int)childID;
		obj["newParentID"] = (int)newParentID;
		insertJson(obj);
	}

	void FilePlotter::insertJson(const QJsonValue& value)
	{
		if(!m_file)
			return;
		
		QTextStream out(m_file);

		// Remove the QJsonArray closing bracket to add the new object
		out.seek(m_file->size() - 5);
		// remove the closing bracket
		out << "";

		// Add a comma to separate the objects if there are already objects in the file
		if (m_file->size() > 3)
		{
			out << ",\n";
		}
		QJsonObject obj;
		if(value.isObject())
		{
			obj = value.toObject();
		}
		else
		{
			obj["value"] = value;
		}

		// Add the new object
		out << QJsonDocument(obj).toJson();

		// Add the closing bracket back
		out << "]\n";
	}
	void FilePlotter::createDirectoryIfNotExists(const QString& filePath)
	{
		// remove file name from path
		QString path = QFileInfo(filePath).absolutePath();
		QDir dir(path);
		if (dir.exists()) {
			//qDebug() << "Directory already exists:" << path;
			return;
		}

		if (dir.mkpath(".")) {
			//qDebug() << "Directory created successfully:" << path;
			return;
		}
	}
}