#include "DynamicNYCTFeedService.h"

#include "SimpleHTTPRequest.h"

namespace nyctlib {
	void DynamicNYCTFeedService::update() {

	}

	std::shared_ptr<NYCTFeedParser> DynamicNYCTFeedService::getLatestFeed() {
		long long ts = time(NULL);

		std::string file_path = "gtfs_nycts_" + std::to_string(ts) + ".bin";

		// DO NOT COMMIT THIS !@!@!@!@! 
		const char *apiendpoint = "http://datamine.mta.info/mta_esi.php?key=4c10dbf5a8889267b4f1a52530f6af6d&feed_id=1";
			//getenv("MTANYCT_API_DEMOURL");//

		try {
			SimpleHTTPRequest requester;
			bool ret = requester.get_save(apiendpoint, file_path.c_str());
			if (!ret)
				throw std::exception(); // yes i am an idiot for doing this
		} catch (std::exception &ex) {
			fprintf(stderr, "Failed to update NYCT feed!\n");
			return nullptr;
		}

		auto cleanup = [&](int reason = 0) {
			// lovely simple c apis
			//printf("Would delete %s\n", file_path.c_str());
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

		this->latest_feed_stamp = new_feed_time;

		return parser;
	}

	std::shared_ptr<NYCTFeedParser> DynamicNYCTFeedService::getCurrentFeed() {
		return getLatestFeed();
	}
}