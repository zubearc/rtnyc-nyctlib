#pragma once

#include "Globals.h"
#include "IFeedService.h"

#ifdef _EMSCRIPTEN

#include <emscripten/val.h>

namespace nyctlib {
	class EmscriptenFeedService : IFeedService {
		EmscriptenFeedService();
	};
}

#endif