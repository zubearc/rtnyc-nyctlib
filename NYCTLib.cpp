// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "gtfs/GtfsFeedParser.h"
#include "gtfs/NYCTFeedParser.h"
#include "NYCTFeedTracker.h"
#include "NYCTFeedService.h"
#include "DynamicNYCTFeedService.h"

#include <stdlib.h>

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	/*
	nyctlib::NYCTFeedParser gtfsFeedParser;
#ifdef _WIN32
	gtfsFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin");
#else
	gtfsFeedParser.loadFile("../res/gtfs_nyct_06192018_0831PM.bin");
#endif
	gtfsFeedParser.dumpOut();*/

	auto feedService = std::unique_ptr<nyctlib::IFeedService>((nyctlib::IFeedService*)new nyctlib::DynamicNYCTFeedService());
	nyctlib::NYCTFeedTracker nyctFeedTracker(std::move(feedService));
	
	//auto trips = nyctFeedTracker.getTripsScheduledToArriveAtStop("217S");
	nyctFeedTracker.printTripsScheduledToArriveAtStop("217S");

	//nyctlib::GtfsFeedParser ferryFeedParser;
	//ferryFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_ferry_tripupdate.bin");

	system("pause");
	return 0;
}
