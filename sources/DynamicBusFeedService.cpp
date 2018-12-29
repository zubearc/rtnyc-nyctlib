#include "DynamicBusFeedService.h"
#include "SimpleHTTPRequest.h"
#include <ctime>

namespace nyctlib {
	void DynamicBusFeedService::update(){
	}

	std::shared_ptr<GtfsFeedParser> DynamicBusFeedService::getLatestFeed() {
		long long ts = time(NULL);

		std::string file_path = "gtfs_nyctb_" + std::to_string(ts) + ".bin";

		try {
			SimpleHTTPRequest requester;
			bool ret = requester.get_save(feed_endpoint.c_str(), file_path.c_str());
			if (!ret)
				throw std::exception(); // yes i am an idiot for doing this
		} catch (std::exception &ex) {
			fprintf(stderr, "DynamicBusFeedService: Failed to update bus feed!\n");
			return nullptr;
		}

		auto cleanup = [&](int reason = 0) {
			// lovely simple c apis
			//printf("Would delete %s\n", file_path.c_str
			printf("Deleting %s: %d\n", file_path.c_str(), remove(file_path.c_str()));
		};

		auto parser = std::make_shared<GtfsFeedParser>();

		if (!parser->loadFile(file_path)) {
			printf("DynamicBusFeedService: Failed to read buffer!!\n");
			cleanup();
			return nullptr;
		}

		auto new_feed_time = parser->getFeedTime();

		auto _last_feed_time = this->latest_feed_stamp;

		if (new_feed_time < _last_feed_time) {
			printf("DynamicBusFeedService: Somehow got a feed TS (%lld) which is older than our previous feed TS (%lld)?!\n", new_feed_time, _last_feed_time);
			cleanup();
			return nullptr;
		}

		if (new_feed_time == _last_feed_time) {
			printf("DynamicBusFeedService: Feed data has same TS as last TS (%lld).\n", new_feed_time);
			cleanup();
			return nullptr;
		}

#ifdef ALWAYS_CLEAN
		printf("DynamicNYCTFeedService: Cleaning because built with ALWAYS_CLEAN\n");
		cleanup();
#endif

		this->latest_feed_stamp = new_feed_time;

		return parser;
	}

	std::shared_ptr<GtfsFeedParser> DynamicBusFeedService::getCurrentFeed() {
		long long current_ts = time(NULL);
		if ((latest_feed_stamp > 0) && ((current_ts - latest_feed_stamp) < 10)) {
			printf("DynamicBusFeedService: Not getting new feed as last was updated < 10s ago (%lld - %lld)\n",
				this->latest_feed_stamp, current_ts);
			return nullptr;
		}
		return getLatestFeed();
	}
}