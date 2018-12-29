#include "IFeedService.h"

namespace nyctlib {

	class DynamicBusFeedService : public IFeedService<GtfsFeedParser> {
		long long latest_feed_stamp = 0;
		std::string feed_endpoint;

	public:
		DynamicBusFeedService(std::string feed_endpoint) {
			this->feed_endpoint = feed_endpoint;
		}

		virtual void update();
		virtual std::shared_ptr<GtfsFeedParser> getCurrentFeed();
		virtual std::shared_ptr<GtfsFeedParser> getLatestFeed();
	};

}