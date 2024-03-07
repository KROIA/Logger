#pragma once

#include <chrono>

#define LOGGER_QT

#ifdef BUILD_STATIC
#define LOGGER_STATIC
#endif

#ifndef LOGGER_STATIC
	#if defined(LOGGER_LIB)
		#define LOGGER_EXPORT __declspec(dllexport)
	#else
		#define LOGGER_EXPORT __declspec(dllimport)
	#endif
#else
	#define LOGGER_EXPORT
#endif


// MSVC Compiler
#ifdef _MSC_VER 
	#define __PRETTY_FUNCTION__ __FUNCSIG__
	typedef std::chrono::steady_clock::time_point TimePoint;
#else
	typedef std::chrono::system_clock::time_point TimePoint;
#endif
