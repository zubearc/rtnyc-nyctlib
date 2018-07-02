#pragma once

#include "Globals.h"
#include "IFeedService.h"

#ifdef _EMSCRIPTEN

#include <emscripten/val.h>

namespace nyctlib {
	class EmscriptenNYCTFeedService : public IFeedService {
public:
		virtual void update() {};

		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed();

		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed() {
			return this->getLatestFeed();
		};
	};
}

#endif