#include "NYCTFeedService.h"

namespace nyctlib {
#ifdef _EMSCRIPTEN
	void NYCTFeedService::updateFromBuffer(const char *buffer) {
		if (!this->feed_parser)
			feed_parser = std::make_shared<NYCTFeedParser>();
		feed_parser->loadBuffer(std::string(buffer));
	}
#endif

	std::shared_ptr<NYCTFeedParser> NYCTFeedService::getLatestFeed() {
		auto parser = std::make_shared<NYCTFeedParser>();
		parser->loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin");
		return parser;
	}
}