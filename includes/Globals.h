#pragma once

#ifdef __EMSCRIPTEN__
#define _EMSCRIPTEN
#endif

#ifndef _ASSERT
#include <cassert>
#define _ASSERT assert
#endif


enum {
	LogDebug = 0b001,
	LogInfo = 0b010,
	LogWarn = 0b100
};


extern int g_logging_level;
extern FILE *g_logging_output_pointer;