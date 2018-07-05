#pragma once

#include "IFeedService.h"

namespace nyctlib {
	class DynamicNYCTFeedService : IFeedService {
	private:
		long long latest_feed_stamp = 0;
	public:
		virtual void update();
		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed();
		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed();
	};
}
