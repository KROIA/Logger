#pragma once

// Profiling
#ifdef LOGGER_PROFILING
#include <easy/profiler.h>
#include <easy/arbitrary_value.h> // EASY_VALUE, EASY_ARRAY are defined here

#define LP_PROFILING_BLOCK_C(text, color) EASY_BLOCK(text, color)
#define LP_PROFILING_NONSCOPED_BLOCK_C(text, color) EASY_NONSCOPED_BLOCK(text, color)
#define LP_PROFILING_END_BLOCK EASY_END_BLOCK
#define LP_PROFILING_FUNCTION_C(color) EASY_FUNCTION(color)
#define LP_PROFILING_BLOCK(text, colorStage) LP_PROFILING_BLOCK_C(text,profiler::colors::  colorStage)
#define LP_PROFILING_NONSCOPED_BLOCK(text, colorStage) LP_PROFILING_NONSCOPED_BLOCK_C(text,profiler::colors::  colorStage)
#define LP_PROFILING_FUNCTION(colorStage) LP_PROFILING_FUNCTION_C(profiler::colors:: colorStage)
#define LP_PROFILING_THREAD(name) EASY_THREAD(name)

#define LP_PROFILING_VALUE(name, value) EASY_VALUE(name, value)
#define LP_PROFILING_TEXT(name, value) EASY_TEXT(name, value)

#else
#define LP_PROFILING_BLOCK_C(text, color)
#define LP_PROFILING_NONSCOPED_BLOCK_C(text, color)
#define LP_PROFILING_END_BLOCK
#define LP_PROFILING_FUNCTION_C(color)
#define LP_PROFILING_BLOCK(text, colorStage)
#define LP_PROFILING_NONSCOPED_BLOCK(text, colorStage)
#define LP_PROFILING_FUNCTION(colorStage)
#define LP_PROFILING_THREAD(name)

#define LP_PROFILING_VALUE(name, value)
#define LP_PROFILING_TEXT(name, value)
#endif

// Special expantion tecniques are required to combine the color name
#define CONCAT_SYMBOLS_IMPL(x, y) x##y
#define CONCAT_SYMBOLS(x, y) CONCAT_SYMBOLS_IMPL(x, y)

// Different color stages
#define LP_COLOR_STAGE_1 50
#define LP_COLOR_STAGE_2 100
#define LP_COLOR_STAGE_3 200
#define LP_COLOR_STAGE_4 300
#define LP_COLOR_STAGE_5 400
#define LP_COLOR_STAGE_6 500
#define LP_COLOR_STAGE_7 600
#define LP_COLOR_STAGE_8 700
#define LP_COLOR_STAGE_9 800
#define LP_COLOR_STAGE_10 900
#define LP_COLOR_STAGE_11 A100 
#define LP_COLOR_STAGE_12 A200 
#define LP_COLOR_STAGE_13 A400 
#define LP_COLOR_STAGE_14 A700 

// General
#define LP_GENERAL_PROFILING_COLORBASE Cyan
#define LP_GENERAL_PROFILING_BLOCK_C(text, color) LP_PROFILING_BLOCK_C(text, color)
#define LP_GENERAL_PROFILING_NONSCOPED_BLOCK_C(text, color) LP_PROFILING_NONSCOPED_BLOCK_C(text, color)
#define LP_GENERAL_PROFILING_END_BLOCK LP_PROFILING_END_BLOCK;
#define LP_GENERAL_PROFILING_FUNCTION_C(color) LP_PROFILING_FUNCTION_C(color)
#define LP_GENERAL_PROFILING_BLOCK(text, colorStage) LP_PROFILING_BLOCK(text, CONCAT_SYMBOLS(LP_GENERAL_PROFILING_COLORBASE, colorStage))
#define LP_GENERAL_PROFILING_NONSCOPED_BLOCK(text, colorStage) LP_PROFILING_NONSCOPED_BLOCK(text, CONCAT_SYMBOLS(LP_GENERAL_PROFILING_COLORBASE, colorStage))
#define LP_GENERAL_PROFILING_FUNCTION(colorStage) LP_PROFILING_FUNCTION(CONCAT_SYMBOLS(LP_GENERAL_PROFILING_COLORBASE, colorStage))
#define LP_GENERAL_PROFILING_VALUE(name, value) LP_PROFILING_VALUE(name, value)
#define LP_GENERAL_PROFILING_TEXT(name, value) LP_PROFILING_TEXT(name, value)


