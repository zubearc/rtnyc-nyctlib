#pragma once

#ifdef __EMSCRIPTEN__
#define _EMSCRIPTEN
#endif

#ifndef _ASSERT
#include <cassert>
#define _ASSERT assert
#endif

#include <cstdio>

enum {
	LogDebug = 0b001,
	LogInfo = 0b010,
	LogWarn = 0b100
};


extern int g_logging_level;
extern FILE *g_logging_output_pointer;

#define INYCTFeedService IFeedService<NYCTFeedParser>
#define INYCTFeedServicePtr std::unique_ptr<IFeedService<NYCTFeedParser>>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// Sleep for a given number of milliseconds
#define SLEEP Sleep
#else
#include <unistd.h>
// Sleep for a given number of milliseconds
#define SLEEP(x) usleep(x * 1000)
#endif

#ifndef min
// min,max functions -- defined in Windows.h on win32
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif