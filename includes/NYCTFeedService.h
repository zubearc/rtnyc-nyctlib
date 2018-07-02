#pragma once

#include "gtfs/NYCTFeedParser.h"
#include "Globals.h"
#include "IFeedService.h"

namespace nyctlib {
	class NYCTFeedService : public IFeedService {
#ifdef  _EMSCRIPTEN
		std::string buffer;
		std::shared_ptr<NYCTFeedParser> feed_parser;
#endif //  _EMSCRIPTEN

	public:
		NYCTFeedService() {

		}

#ifdef _EMSCRIPTEN
		void updateFromBuffer(const char *buffer);
#endif
		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed();

		virtual void update() {}; // TODO: stub

		// Gets latest cached feed
		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed() {
			return getLatestFeed();
		}
	};
}