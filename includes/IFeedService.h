#pragma once

#include "gtfs/NYCTFeedParser.h"

namespace nyctlib {

	template <typename TFeedParser>
	class IFeedService {
		public:
		virtual void update() = 0;
		virtual std::shared_ptr<TFeedParser> getCurrentFeed() = 0;
		virtual std::shared_ptr<TFeedParser> getLatestFeed() = 0;
#ifdef _EMSCRIPTEN
		virtual void updateFromBuffer(const char *buffer, int length) {};
#endif
	};

}