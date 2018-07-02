#pragma once

#include "gtfs/NYCTFeedParser.h"

namespace nyctlib {

	class IFeedService {
		public:
		virtual void update() = 0;
		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed() = 0;
		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed() = 0;
	};

}