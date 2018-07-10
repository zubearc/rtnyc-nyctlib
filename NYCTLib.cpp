// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "gtfs/GtfsFeedParser.h"
#include "gtfs/NYCTFeedParser.h"
#include "NYCTFeedTracker.h"
#include "NYCTFeedService.h"
#include "DynamicNYCTFeedService.h"
#include "interfaces/ConsoleInterface.h"

#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#endif

int g_logging_level = LogWarn;
FILE *g_logging_output_pointer = stdout;

using namespace std;
using namespace nyctlib;

int main(int argc, char *argv[])
{
	cout << "Hello CMake." << endl;

#ifdef _WIN32 // enable colors
	// enable ANSI sequences for windows 10:
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode(console, &consoleMode);
	consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(console, consoleMode);
#endif

	/*
	nyctlib::NYCTFeedParser gtfsFeedParser;
#ifdef _WIN32
	gtfsFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin");
#else
	gtfsFeedParser.loadFile("../res/gtfs_nyct_06192018_0831PM.bin");
#endif
	gtfsFeedParser.dumpOut();*/

	if (argc == 1) {
		auto feedService = std::unique_ptr<nyctlib::IFeedService>((nyctlib::IFeedService*)new nyctlib::DynamicNYCTFeedService());
		nyctlib::NYCTFeedTracker nyctFeedTracker(std::move(feedService));

		//auto trips = nyctFeedTracker.getTripsScheduledToArriveAtStop("217S");
		nyctFeedTracker.run();
		nyctFeedTracker.printTripsScheduledToArriveAtStop("217S");

		//nyctlib::GtfsFeedParser ferryFeedParser;
		//ferryFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_ferry_tripupdate.bin");
//		nyctlib::GtfsFeedParser busFeedParser;
//		busFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCTBus/DataArchives/tripUpdates");
	} else {
		ConsoleInterface ci;
		ci.run(argc, argv);
	}

	system("pause");
	return 0;
}
