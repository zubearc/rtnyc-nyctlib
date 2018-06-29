#pragma once

#include "gtfs/NYCTFeedParser.h"

namespace nyctlib {
	class NYCTFeedService {
	public:
		NYCTFeedService() {

		}

		std::shared_ptr<NYCTFeedParser> getLatestFeed();

		// Gets latest cached feed
		std::shared_ptr<NYCTFeedParser> getCurrentFeed() {
			return getLatestFeed();
		}
	};
}