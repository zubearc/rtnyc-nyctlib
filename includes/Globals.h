#pragma once

#ifdef __EMSCRIPTEN__
#define _EMSCRIPTEN
#endif

#ifndef _ASSERT
#include <cassert>
#define _ASSERT assert
#endif