#pragma once

#include <cstdio>
#include "Globals.h"

/* Logging Color Macros */
#define CH_GREEN "\33[0;92m"
#define CH_RED "\33[0;91m"
#define CH_YELLOW "\33[0;93m"
#define CH_MAGENTA "\33[95m"
#define CH_BLUE "\33[1;94m"
#define CPURPLE "\33[0;35m"
#define CGREEN "\33[0;32m"
#define CYELLOW "\33[0;33m"
#define CMAGENTA "\33[35m"
#define CWHITE "\33[1;37m"
#define CBLUE "\33[0;34m"
#define CRESET "\33[0m"

#define LOG_RAW_DEBUG(...) if ((g_logging_level & LogDebug) > 0) fprintf(g_logging_output_pointer, __VA_ARGS__);
#define LOG_RAW_INFO(...) if ((g_logging_level & LogInfo) > 0) fprintf(g_logging_output_pointer, __VA_ARGS__);

#ifdef _WIN32
// thank you microsoft for not being standards-compliant
// https://stackoverflow.com/a/17837382
template <typename ...Args>
void LOG_FT_DEBUG(const char *text, Args... args) {
	if ((g_logging_level & LogDebug) > 0) {
		fprintf(g_logging_output_pointer, "DEBUG ");
		fprintf(g_logging_output_pointer, text, args...);
	}
}
void LOG_FT_DEBUG(const char *text) {
	if ((g_logging_level & LogDebug) > 0) {
		fprintf(g_logging_output_pointer, "DEBUG ");
		fprintf(g_logging_output_pointer, text);
	}
}
template <typename ...Args>
void LOG_FT_INFO(const char *text, Args... args) {
	if ((g_logging_level & LogInfo) > 0) {
		fprintf(g_logging_output_pointer, "INFO ");
		fprintf(g_logging_output_pointer, text, args...);
	}
}
void LOG_FT_INFO(const char *text) {
	if ((g_logging_level & LogInfo) > 0) {
		fprintf(g_logging_output_pointer, "INFO ");
		fprintf(g_logging_output_pointer, text);
	}
}
template <typename ...Args>
void LOG_FT_WARN(const char *text, Args ...args) {
	if ((g_logging_level & LogWarn) > 0) {
		fprintf(stderr, "WARN ");
		fprintf(stderr, text, args...);
	}
}
void LOG_FT_WARN(const char *text) {
	if ((g_logging_level & LogWarn) > 0) {
		fprintf(stderr, "WARN ");
		fprintf(stderr, text);
	}
}
#else
#define LOG_FT_DEBUG(...) if ((g_logging_level & LogDebug) > 0) fprintf(g_logging_output_pointer, "DEBUG " __VA_ARGS__)
#define LOG_FT_INFO(...) if ((g_logging_level & LogInfo) > 0) fprintf(g_logging_output_pointer, "INFO " __VA_ARGS__)
#define LOG_FT_WARN(...) if ((g_logging_level & LogWarn) > 0) fprintf(stderr, "WARN " __VA_ARGS__)
#endif
