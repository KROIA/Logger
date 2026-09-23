#include "FilePlotter.h"
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include "Utilities/Export.h"

namespace
{
	// Without this, Qt falls back to the locale codec (CP1252 on a German Windows box)
	// and every non-ASCII character is irreversibly lost on disk.
	void setUtf8(QTextStream& out)
	{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		out.setCodec("UTF-8");
#else
		out.setEncoding(QStringConverter::Utf8);
#endif
	}
}

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
			m_stream.setDevice(m_file);
			setUtf8(m_stream);
			m_stream << "[\n";
			m_stream << QJsonDocument(Export::getFileHeader()).toJson();
			m_stream << "]\n";
			m_stream.flush();
		}
	}
	FilePlotter::~FilePlotter()
	{
		if (m_file)
		{
			m_stream.flush();
			m_stream.setDevice(nullptr);
			m_file->close();
			delete m_file;
		}
	}
	void FilePlotter::onNewLogger(LogObject::Info loggerInfo)
	{
		LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
		insertJson(loggerInfo.toJson());
	}
	void FilePlotter::onLoggerInfoChanged(LogObject::Info info)
	{
		LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
		insertJson(info.toJson());
	}
	void FilePlotter::onLogMessage(Message message)
	{
		LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
		insertJson(message.toJson());
	}
	void FilePlotter::onChangeParent(LoggerID childID, LoggerID newParentID)
	{
		LOGGER_RECEIVER_PROFILING_FUNCTION(LOGGER_COLOR_STAGE_1);
		QJsonObject obj;
		obj["childID"] = (int)childID;
		obj["newParentID"] = (int)newParentID;
		insertJson(obj);
	}

	void FilePlotter::insertJson(const QJsonValue& value)
	{
		if(!m_file)
			return;

		QTextStream& out = m_stream;
		// The stream buffers writes, so the device size is only trustworthy
		// after a flush — and the seek below is computed from it.
		out.flush();

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
		out.flush();
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