#pragma once
#include "Logger_global.h"

/// USER_SECTION_START 14
/*
/// USER_SECTION_END

// The Logger library is automaticly included if the logger dependency .cmake file is available
#if LOGGER_LIBRARY_AVAILABLE == 1
	#include "Logger.h"
#endif

/// USER_SECTION_START 1
*/
/// USER_SECTION_END

// Debugging
#ifdef NDEBUG
	#define LOGGER_CONSOLE(msg)
	#define LOGGER_CONSOLE_FUNCTION(msg)
#else
	#include <iostream>

	#define LOGGER_DEBUG
	#define LOGGER_CONSOLE_STREAM std::cout

	#define LOGGER_CONSOLE(msg) LOGGER_CONSOLE_STREAM << msg;
	#define LOGGER_CONSOLE_FUNCTION(msg) LOGGER_CONSOLE_STREAM << __PRETTY_FUNCTION__ << " " << msg;
#endif

/// USER_SECTION_START 2
#if defined(_DEBUG) && defined(LOGGER_ENABLE_DEBUG_BREAK)
	#if defined(_MSC_VER)  // If using Microsoft Visual Studio
		#define LOGGER_DEBUG_BREAK __debugbreak()
	#elif defined(__GNUC__) || defined(__clang__)  // If using GCC or Clang
		#include <signal.h>
		#define LOGGER_DEBUG_BREAK raise(SIGTRAP)
	#else
		#define LOGGER_DEBUG_BREAK 
	#endif

	#if defined(LOGGER_ENABLE_DEBUG_BREAK_ON_ERROR) && !defined(LOGGER_DISABLE_DEBUG_BREAK_ON_ERROR)
		#define LOGGER_DEBUG_BREAK_ON_ERROR(msg) \
			if(msg.getLevel() == Log::Level::error) \
			{ \
				std::string errorMessage = msg.getText(); \
                LOGGER_UNUSED(errorMessage); \
				LOGGER_DEBUG_BREAK; \
			} 
	#else
		#define LOGGER_DEBUG_BREAK_ON_ERROR(msg)
	#endif
#else
	#define LOGGER_DEBUG_BREAK
	#define LOGGER_DEBUG_BREAK_ON_ERROR(msg)
#endif


/// USER_SECTION_END

#ifdef LOGGER_PROFILING
	#include "easy/profiler.h"
	#include <easy/arbitrary_value.h> // EASY_VALUE, EASY_ARRAY are defined here

	#define LOGGER_PROFILING_BLOCK_C(text, color) EASY_BLOCK(text, color)
	#define LOGGER_PROFILING_NONSCOPED_BLOCK_C(text, color) EASY_NONSCOPED_BLOCK(text, color)
	#define LOGGER_PROFILING_END_BLOCK EASY_END_BLOCK
	#define LOGGER_PROFILING_FUNCTION_C(color) EASY_FUNCTION(color)
	#define LOGGER_PROFILING_BLOCK(text, colorStage) LOGGER_PROFILING_BLOCK_C(text,profiler::colors::  colorStage)
	#define LOGGER_PROFILING_NONSCOPED_BLOCK(text, colorStage) LOGGER_PROFILING_NONSCOPED_BLOCK_C(text,profiler::colors::  colorStage)
	#define LOGGER_PROFILING_FUNCTION(colorStage) LOGGER_PROFILING_FUNCTION_C(profiler::colors:: colorStage)
	#define LOGGER_PROFILING_THREAD(name) EASY_THREAD(name)

	#define LOGGER_PROFILING_VALUE(name, value) EASY_VALUE(name, value)
	#define LOGGER_PROFILING_TEXT(name, value) EASY_TEXT(name, value)

#else
	#define LOGGER_PROFILING_BLOCK_C(text, color)
	#define LOGGER_PROFILING_NONSCOPED_BLOCK_C(text, color)
	#define LOGGER_PROFILING_END_BLOCK
	#define LOGGER_PROFILING_FUNCTION_C(color)
	#define LOGGER_PROFILING_BLOCK(text, colorStage)
	#define LOGGER_PROFILING_NONSCOPED_BLOCK(text, colorStage)
	#define LOGGER_PROFILING_FUNCTION(colorStage)
	#define LOGGER_PROFILING_THREAD(name)

	#define LOGGER_PROFILING_VALUE(name, value)
	#define LOGGER_PROFILING_TEXT(name, value)
#endif

// Special expantion tecniques are required to combine the color name
#define CONCAT_SYMBOLS_IMPL(x, y) x##y
#define CONCAT_SYMBOLS(x, y) CONCAT_SYMBOLS_IMPL(x, y)



// Different color stages
#define LOGGER_COLOR_STAGE_1 50
#define LOGGER_COLOR_STAGE_2 100
#define LOGGER_COLOR_STAGE_3 200
#define LOGGER_COLOR_STAGE_4 300
#define LOGGER_COLOR_STAGE_5 400
#define LOGGER_COLOR_STAGE_6 500
#define LOGGER_COLOR_STAGE_7 600
#define LOGGER_COLOR_STAGE_8 700
#define LOGGER_COLOR_STAGE_9 800
#define LOGGER_COLOR_STAGE_10 900
#define LOGGER_COLOR_STAGE_11 A100 
#define LOGGER_COLOR_STAGE_12 A200 
#define LOGGER_COLOR_STAGE_13 A400 
#define LOGGER_COLOR_STAGE_14 A700 

namespace Log
{
/// USER_SECTION_START 4
	
/// USER_SECTION_END
	class LOGGER_EXPORT Profiler
	{
	public:
		/// USER_SECTION_START 5

		/// USER_SECTION_END

		// Implementation defined in LibraryName_info.cpp to save files.
		static void start();
		static void stop();
		static void stop(const char* profilerOutputFile);

		/// USER_SECTION_START 6

		/// USER_SECTION_END
	};

/// USER_SECTION_START 7
/*
* Disable the Logger interface because this is the Logger library itself.
/// USER_SECTION_END


#if LOGGER_LIBRARY_AVAILABLE == 1
	class LOGGER_EXPORT Logger 
	{
		/// USER_SECTION_START 8

		/// USER_SECTION_END
		Logger();
		static Logger& instance();
		public:
		/// USER_SECTION_START 9

		/// USER_SECTION_END

		static void setEnabled(bool enable);
		static bool isEnabled();
		static void setName(const std::string& name);
		static std::string getName();
		static void setColor(const Log::Color& col);
		static Log::Color getColor();
		static Log::DateTime getCreationDateTime();
		static Log::LoggerID getID();
		static void setParentID(Log::LoggerID parentID);
		static Log::LoggerID getParentID();



		static void log(const Log::Message& msg);

		static void log(const std::string& msg);
		static void log(const std::string& msg, Log::Level level);
		static void log(const std::string& msg, Log::Level level, const Log::Color& col);

		static void logTrace(const std::string& msg);
		static void logDebug(const std::string& msg);
		static void logInfo(const std::string& msg);
		static void logWarning(const std::string& msg);
		static void logError(const std::string& msg);
		static void logCustom(const std::string& msg);

		/// USER_SECTION_START 10

		/// USER_SECTION_END

		private:
		Log::LogObject m_logObject;

		/// USER_SECTION_START 11

		/// USER_SECTION_END
	};
/// USER_SECTION_START 12

/// USER_SECTION_END
#endif
/// USER_SECTION_START 13
*/
/// USER_SECTION_END
}


// General
#define LOGGER_GENERAL_PROFILING_COLORBASE Cyan
#define LOGGER_GENERAL_PROFILING_BLOCK_C(text, color) LOGGER_PROFILING_BLOCK_C(text, color)
#define LOGGER_GENERAL_PROFILING_NONSCOPED_BLOCK_C(text, color) LOGGER_PROFILING_NONSCOPED_BLOCK_C(text, color)
#define LOGGER_GENERAL_PROFILING_END_BLOCK LOGGER_PROFILING_END_BLOCK;
#define LOGGER_GENERAL_PROFILING_FUNCTION_C(color) LOGGER_PROFILING_FUNCTION_C(color)
#define LOGGER_GENERAL_PROFILING_BLOCK(text, colorStage) LOGGER_PROFILING_BLOCK(text, CONCAT_SYMBOLS(LOGGER_GENERAL_PROFILING_COLORBASE, colorStage))
#define LOGGER_GENERAL_PROFILING_NONSCOPED_BLOCK(text, colorStage) LOGGER_PROFILING_NONSCOPED_BLOCK(text, CONCAT_SYMBOLS(LOGGER_GENERAL_PROFILING_COLORBASE, colorStage))
#define LOGGER_GENERAL_PROFILING_FUNCTION(colorStage) LOGGER_PROFILING_FUNCTION(CONCAT_SYMBOLS(LOGGER_GENERAL_PROFILING_COLORBASE, colorStage))
#define LOGGER_GENERAL_PROFILING_VALUE(name, value) LOGGER_PROFILING_VALUE(name, value)
#define LOGGER_GENERAL_PROFILING_TEXT(name, value) LOGGER_PROFILING_TEXT(name, value)


/// USER_SECTION_START 3
#define LOGGER_SENDER_PROFILING_COLORBASE Orange
#define LOGGER_SENDER_PROFILING_BLOCK_C(text, color) LOGGER_PROFILING_BLOCK_C(text, color)
#define LOGGER_SENDER_PROFILING_NONSCOPED_BLOCK_C(text, color) LOGGER_PROFILING_NONSCOPED_BLOCK_C(text, color)
#define LOGGER_SENDER_PROFILING_END_BLOCK LOGGER_PROFILING_END_BLOCK;
#define LOGGER_SENDER_PROFILING_FUNCTION_C(color) LOGGER_PROFILING_FUNCTION_C(color)
#define LOGGER_SENDER_PROFILING_BLOCK(text, colorStage) LOGGER_PROFILING_BLOCK(text, CONCAT_SYMBOLS(LOGGER_SENDER_PROFILING_COLORBASE, colorStage))
#define LOGGER_SENDER_PROFILING_NONSCOPED_BLOCK(text, colorStage) LOGGER_PROFILING_NONSCOPED_BLOCK(text, CONCAT_SYMBOLS(LOGGER_SENDER_PROFILING_COLORBASE, colorStage))
#define LOGGER_SENDER_PROFILING_FUNCTION(colorStage) LOGGER_PROFILING_FUNCTION(CONCAT_SYMBOLS(LOGGER_SENDER_PROFILING_COLORBASE, colorStage))
#define LOGGER_SENDER_PROFILING_VALUE(name, value) LOGGER_PROFILING_VALUE(name, value)
#define LOGGER_SENDER_PROFILING_TEXT(name, value) LOGGER_PROFILING_TEXT(name, value)

#define LOGGER_RECEIVER_PROFILING_COLORBASE Yellow
#define LOGGER_RECEIVER_PROFILING_BLOCK_C(text, color) LOGGER_PROFILING_BLOCK_C(text, color)
#define LOGGER_RECEIVER_PROFILING_NONSCOPED_BLOCK_C(text, color) LOGGER_PROFILING_NONSCOPED_BLOCK_C(text, color)
#define LOGGER_RECEIVER_PROFILING_END_BLOCK LOGGER_PROFILING_END_BLOCK;
#define LOGGER_RECEIVER_PROFILING_FUNCTION_C(color) LOGGER_PROFILING_FUNCTION_C(color)
#define LOGGER_RECEIVER_PROFILING_BLOCK(text, colorStage) LOGGER_PROFILING_BLOCK(text, CONCAT_SYMBOLS(LOGGER_RECEIVER_PROFILING_COLORBASE, colorStage))
#define LOGGER_RECEIVER_PROFILING_NONSCOPED_BLOCK(text, colorStage) LOGGER_PROFILING_NONSCOPED_BLOCK(text, CONCAT_SYMBOLS(LOGGER_RECEIVER_PROFILING_COLORBASE, colorStage))
#define LOGGER_RECEIVER_PROFILING_FUNCTION(colorStage) LOGGER_PROFILING_FUNCTION(CONCAT_SYMBOLS(LOGGER_RECEIVER_PROFILING_COLORBASE, colorStage))
#define LOGGER_RECEIVER_PROFILING_VALUE(name, value) LOGGER_PROFILING_VALUE(name, value)
#define LOGGER_RECEIVER_PROFILING_TEXT(name, value) LOGGER_PROFILING_TEXT(name, value)

/// USER_SECTION_END