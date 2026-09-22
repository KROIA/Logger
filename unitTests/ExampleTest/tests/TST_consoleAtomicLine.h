#pragma once

#include "UnitTest.h"
#include "Logger.h"
#include <QCoreApplication>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>


// ISS-004: with atomic lines enabled, a log line must reach the stream as a single
// write, so a foreign writer can only interleave between lines, never inside one.
class TST_consoleAtomicLine : public UnitTest::Test
{
	TEST_CLASS(TST_consoleAtomicLine)
public:
	TST_consoleAtomicLine()
		: Test("TST_consoleAtomicLine")
	{
		ADD_TEST(TST_consoleAtomicLine::line_is_one_write);
		ADD_TEST(TST_consoleAtomicLine::colored_line_is_one_write);
		ADD_TEST(TST_consoleAtomicLine::colors_off_emits_no_escapes);
	}

private:

	// Records every chunk handed to the stream, so the number of writes is observable.
	class ChunkRecorder : public std::streambuf
	{
	public:
		std::vector<std::string> chunks;
	protected:
		std::streamsize xsputn(const char* s, std::streamsize n) override
		{
			chunks.push_back(std::string(s, (size_t)n));
			return n;
		}
		int overflow(int c) override
		{
			if (c != EOF)
				chunks.push_back(std::string(1, (char)c));
			return c;
		}
	};

	// Logs one message through an atomic-line view with std::cout redirected, and returns
	// every recorded write that carried the message text.
	static std::vector<std::string> capture(const std::string& loggerName,
										   const std::string& text,
										   bool colors,
										   const Log::Color& loggerColor = Log::Color(255, 255, 255))
	{
		ChunkRecorder recorder;
		std::streambuf* old = std::cout.rdbuf(&recorder);

		{
			Log::UI::NativeConsoleView view;
			view.setAtomicLines(true);
			view.setAtomicLineColors(colors);
			Log::LogObject logger(loggerName);
			logger.setColor(loggerColor);
			logger.log(text);
			QCoreApplication::processEvents();
		}

		std::cout.rdbuf(old);

		std::vector<std::string> hits;
		for (const std::string& chunk : recorder.chunks)
		{
			if (chunk.find(text) != std::string::npos)
				hits.push_back(chunk);
		}
		return hits;
	}

	TEST_FUNCTION(line_is_one_write)
	{
		TEST_START;

		const std::vector<std::string> hits = capture("atomicTest", "needle", false);
		TEST_ASSERT_M(hits.size() == 1, "the message must be emitted by exactly one write");

		const std::string& line = hits[0];
		// The whole line, logger name through trailing newline, in this one chunk.
		TEST_ASSERT_M(line.find("atomicTest") != std::string::npos,
					  "logger name must be in the same write as the message");
		TEST_ASSERT_M(!line.empty() && line.back() == '\n', "the write must end the line");
	}

	// Colour must survive the single write: the escapes travel inside the string.
	TEST_FUNCTION(colored_line_is_one_write)
	{
		TEST_START;

		const Log::Color loggerColor(12, 34, 56);
		const std::vector<std::string> hits = capture("coloredTest", "needle", true, loggerColor);
		TEST_ASSERT_M(hits.size() == 1, "colouring must not split the line into several writes");

		const std::string& line = hits[0];
		TEST_ASSERT_M(line.find("\033[38;2;12;34;56m") != std::string::npos,
					  "the logger colour must be written as a truecolor escape");
		const size_t colorPos = line.find("\033[38;2;12;34;56m");
		TEST_ASSERT_M(colorPos < line.find("coloredTest"),
					  "the colour escape must precede the text it colours");
		TEST_ASSERT_M(line.size() > 4 && line.compare(line.size() - 5, 5, "\033[0m\n") == 0,
					  "the line must reset the colour before the newline");
	}

	// With colours off nothing escapes into the stream - a redirected log stays clean.
	TEST_FUNCTION(colors_off_emits_no_escapes)
	{
		TEST_START;

		const std::vector<std::string> hits = capture("plainTest", "needle", false, Log::Color(12, 34, 56));
		TEST_ASSERT_M(hits.size() == 1, "the message must be emitted by exactly one write");
		TEST_ASSERT_M(hits[0].find('\033') == std::string::npos,
					  "no escape sequence may reach a non-console stream");
	}
};
