#pragma once

#include "gtfs/NYCTFeedParser.h"

namespace nyctlib {

	class IFeedService {
		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed() = 0;
		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed() = 0;
	};

}