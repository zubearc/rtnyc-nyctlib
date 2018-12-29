#pragma once

#include "IFeedService.h"

namespace nyctlib {
	class DynamicNYCTFeedService : public IFeedService<NYCTFeedParser> {
	private:
		long long latest_feed_stamp = 0;
		std::string feed_endpoint;
	public:
		DynamicNYCTFeedService(std::string feed_endpoint) {
			this->feed_endpoint = feed_endpoint;
		}
		virtual void update();
		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed();
		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed();
	};
}
