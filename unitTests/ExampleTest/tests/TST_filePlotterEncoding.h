#pragma once

#include "UnitTest.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>


// ISS-003: FilePlotter must write UTF-8, not the locale codepage.
class TST_filePlotterEncoding : public UnitTest::Test
{
	TEST_CLASS(TST_filePlotterEncoding)
public:
	TST_filePlotterEncoding()
		: Test("TST_filePlotterEncoding")
	{
		ADD_TEST(TST_filePlotterEncoding::umlaut_survives_roundtrip);
	}

private:

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
