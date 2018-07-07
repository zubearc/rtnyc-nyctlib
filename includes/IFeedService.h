#pragma once

#include "gtfs/NYCTFeedParser.h"

namespace nyctlib {

	class IFeedService {
		public:
		virtual void update() = 0;
		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed() = 0;
		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed() = 0;
#ifdef _EMSCRIPTEN
		virtual void updateFromBuffer(const char *buffer, int length) {};
#endif
	};

}