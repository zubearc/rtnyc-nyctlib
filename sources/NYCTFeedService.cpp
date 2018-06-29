#include "NYCTFeedService.h"

namespace nyctlib {
	std::shared_ptr<NYCTFeedParser> NYCTFeedService::getLatestFeed() {
		auto parser = std::make_shared<NYCTFeedParser>();
		parser->loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin");
		return parser;
	}
}