// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "gtfs/GtfsFeedParser.h"
#include "gtfs/NYCTFeedParser.h"
#include "subway/NYCTFeedTracker.h"
#include "NYCTFeedService.h"
#include "ReplayFeedService.h"
#include "DynamicNYCTFeedService.h"
#include "interfaces/ConsoleInterface.h"
#include "interfaces/WSInterface.h"
#include "interfaces/NYCTSubwayInterface.h"
#include "events/EventHolder.h"

#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#endif

int g_logging_level = LogWarn | LogInfo | LogDebug;
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
		
/*		auto rfs = new ReplayFeedService<NYCTFeedParser>("C:/Users/Extreme/CMakeBuilds/9a20cf93-1bae-a730-96f6-0e9e11475965/build/x86-Debug", "gtfs_nycts");
		rfs->jumpTo("gtfs_nycts_1531197614.bin");
		rfs->setMaximumPlaybacks(1);
			
		auto feedService = std::unique_ptr<IFeedService>((IFeedService*)rfs);
*/		
		auto event_holder = std::make_shared<BlockingEventHolder<SubwayTripEvent>>();
		nyctlib::NYCTFeedTracker nyctFeedTracker(std::move(feedService), event_holder);

		//auto trips = nyctFeedTracker.getTripsScheduledToArriveAtStop("217S");
		/*auto memleaktest = std::thread([&]() {
			std::this_thread::sleep_for(std::chrono::seconds(60));
			nyctFeedTracker.kill();
		});
		memleaktest.detach();*/

		auto tracker_running = std::thread([&]() {
			nyctFeedTracker.run();
		});

		tracker_running.detach();
		WSInterface wsi;
		auto subway_ws_interface = NYCTSubwayInterface(&wsi, &nyctFeedTracker, event_holder);

		auto ws_listening_thread = std::thread([&] {
			wsi.start("", 7777);
		});

		auto event_handling_thread = std::thread([&] {
			subway_ws_interface.run();
		});

		std::this_thread::sleep_for(std::chrono::seconds(4000));

		//Sleep(6400);
		//nyctFeedTracker.printTripsScheduledToArriveAtStop("633S");

		//Sleep(60000000000);
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
