#include "DynamicNYCTFeedService.h"

#include "SimpleHTTPRequest.h"

#ifdef _WIN32
#define ALWAYS_CLEAN
#endif

namespace nyctlib {
	void DynamicNYCTFeedService::update() {

	}

	std::shared_ptr<NYCTFeedParser> DynamicNYCTFeedService::getLatestFeed() {
		long long ts = time(NULL);

		std::string file_path = "gtfs_nycts_" + std::to_string(ts) + ".bin";

		try {
			SimpleHTTPRequest requester;
			bool ret = requester.get_save(this->feed_endpoint.c_str(), file_path.c_str(), this->api_key.c_str());
			if (!ret)
				throw std::exception(); // yes i am an idiot for doing this
		} catch (std::exception &ex) {
			fprintf(stderr, "Failed to update NYCT feed!\n");
			return nullptr;
		}

		auto cleanup = [&](int reason = 0) {
			// lovely simple c apis
			//printf("Would delete %s\n", file_path.c_str
			printf("Deleting %s: %d\n", file_path.c_str(), remove(file_path.c_str()));
		};

		auto parser = std::make_shared<NYCTFeedParser>();
		
		if (!parser->loadFile(file_path)) {
			printf("Failed to read buffer!!\n");
			cleanup();
			return nullptr;
		}

		auto new_feed_time = parser->getFeedTime();

		auto _last_feed_time = this->latest_feed_stamp;

		if (new_feed_time < _last_feed_time) {
			printf("DynamicNYCTFeedService.cpp: Somehow got a feed TS (%lld) which is older than our previous feed TS (%lld)?!\n", new_feed_time, _last_feed_time);
			cleanup();
			return nullptr;
		}

		if (new_feed_time == _last_feed_time) {
			printf("DynamicNYCTFeedService.cpp: Feed data has same TS as last TS (%lld).\n", new_feed_time);
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

	std::shared_ptr<NYCTFeedParser> DynamicNYCTFeedService::getCurrentFeed() {
		return getLatestFeed();
	}
}