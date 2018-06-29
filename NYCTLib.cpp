// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "gtfs/GtfsFeedParser.h"
#include "gtfs/NYCTFeedParser.h"
#include "NYCTFeedTracker.h"
#include "NYCTFeedService.h"

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

#endif
	gtfsFeedParser.dumpOut();*/

	nyctlib::NYCTFeedTracker nyctFeedTracker;
	//auto trips = nyctFeedTracker.getTripsScheduledToArriveAtStop("217S");
	nyctFeedTracker.printTripsScheduledToArriveAtStop("217S");

	system("pause");
	return 0;
}
