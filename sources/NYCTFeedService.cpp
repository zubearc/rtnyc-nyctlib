#include "NYCTFeedService.h"

namespace nyctlib {
#ifdef _EMSCRIPTEN
	void NYCTFeedService::updateFromBuffer(const char *buffer) {
		if (!this->feed_parser)
			feed_parser = std::make_shared<NYCTFeedParser>();
		std::string sbuffer(buffer);
		feed_parser->loadBuffer(sbuffer);
	}
#endif

	std::shared_ptr<NYCTFeedParser> NYCTFeedService::getLatestFeed() {
		auto parser = std::make_shared<NYCTFeedParser>();
#ifdef _WIN32
		std::string path = "H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin";
#else
		std::string path = "../res/gtfs_nyct_06192018_0831PM.bin";
#endif
		parser->loadFile(path);
		return parser;
	}
}