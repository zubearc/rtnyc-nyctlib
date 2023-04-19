#pragma once

#include "IFeedService.h"

namespace nyctlib {
	class DynamicNYCTFeedService : public IFeedService<NYCTFeedParser> {
	private:
		long long latest_feed_stamp = 0;
		std::string feed_endpoint;
		std::string api_key;
	public:
		DynamicNYCTFeedService(std::string feed_endpoint, std::string api_key = "") {
			this->feed_endpoint = feed_endpoint;
			this->api_key = api_key;
		}
		virtual void update();
		virtual std::shared_ptr<NYCTFeedParser> getCurrentFeed();
		virtual std::shared_ptr<NYCTFeedParser> getLatestFeed();
	};
}
