#pragma once

#include "UnitTest.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>


// ISS-003: FilePlotter must write UTF-8, not the locale codepage.
class TST_filePlotterEncoding : public UnitTest::Test
{
	TEST_CLASS(TST_filePlotterEncoding)
public:
	TST_filePlotterEncoding()
		: Test("TST_filePlotterEncoding")
	{
		ADD_TEST(TST_filePlotterEncoding::umlaut_survives_roundtrip);
		ADD_TEST(TST_filePlotterEncoding::file_stays_valid_json);
	}

private:

	// The plotter appends by seeking back over the array's closing bracket, and
	// that seek is computed from the device size. Since the QTextStream is now
	// kept open across messages instead of being rebuilt per write, a missing
	// flush would silently corrupt the file. Parse it to prove it did not.
	TEST_FUNCTION(file_stays_valid_json)
	{
		TEST_START;

		static constexpr int kMessages = 25;
		const QString path = QDir::temp().filePath("logger_json_test.json");
		QFile::remove(path);

		{
			Log::FilePlotter plotter(path.toStdString());
			Log::LogObject logger("jsonTest");
			for (int i = 0; i < kMessages; ++i)
				logger.log("json message " + std::to_string(i));
			QCoreApplication::processEvents();
		}

		QFile f(path);
		TEST_ASSERT_M(f.open(QIODevice::ReadOnly), "log file must exist");
		const QByteArray raw = f.readAll();
		f.close();
		QFile::remove(path);

		QJsonParseError error;
		const QJsonDocument doc = QJsonDocument::fromJson(raw, &error);
		TEST_ASSERT_M(error.error == QJsonParseError::NoError,
			"log file must be parseable JSON, got: " + error.errorString().toStdString());
		TEST_ASSERT_M(doc.isArray(), "log file must hold a JSON array");
		// header + one logger registration + one entry per message
		TEST_ASSERT_M(doc.array().size() >= kMessages,
			"every message must be in the file: expected at least " +
			std::to_string(kMessages) + " entries, got " +
			std::to_string(doc.array().size()));
	}

	TEST_FUNCTION(umlaut_survives_roundtrip)
	{
		TEST_START;

		const QString path = QDir::temp().filePath("logger_utf8_test.json");
		QFile::remove(path);

		{
			Log::FilePlotter plotter(path.toStdString());
			Log::LogObject logger("utf8Test"); // receivers subscribe to the LogManager globally
			logger.log("f\xC3\xBCg"); // UTF-8 "füg"
			QCoreApplication::processEvents();
		}

		QFile f(path);
		TEST_ASSERT_M(f.open(QIODevice::ReadOnly), "log file must exist");
		const QByteArray raw = f.readAll();
		f.close();
		QFile::remove(path);

		TEST_ASSERT_M(raw.contains("f\xC3\xBCg"), "umlaut must be on disk as UTF-8 (C3 BC)");
		TEST_ASSERT_M(!raw.contains("f\xFC" "g"), "umlaut must not be written as locale codepage (FC)");
	}
};
