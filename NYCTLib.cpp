// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "gtfs/GtfsFeedParser.h"
#include "gtfs/NYCTFeedParser.h"
#include "subway/NYCTFeedTracker.h"
#include "busses/NYCBusTracker.h"
#include "NYCTFeedService.h"
#include "ReplayFeedService.h"
#include "DynamicNYCTFeedService.h"
#include "DynamicBusFeedService.h"
#include "interfaces/ConsoleInterface.h"
#include "interfaces/WSInterface.h"
#include "interfaces/NYCTSubwayInterface.h"
#include "interfaces/NYCBusInterface.h"
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
		auto apikey = getenv("MTA_NYCT_API_KEY");
		if (apikey == NULL) {
			fprintf(stderr, "fatal: must set MTA_NYCT_API_KEY enviornment var");
			exit(1);
		}
		auto apikey_str = std::string(apikey);
		
		INYCTFeedServicePtr feedService123456 = std::make_unique<DynamicNYCTFeedService>(
			("http://datamine.mta.info/mta_esi.php?key=" + apikey_str +  "&feed_id=1"));
		INYCTFeedServicePtr feedServiceL = std::make_unique<DynamicNYCTFeedService>(
			("http://datamine.mta.info/mta_esi.php?key=" + apikey_str + "&feed_id=2"));
		INYCTFeedServicePtr feedServiceBDFM = std::make_unique<DynamicNYCTFeedService>(
			("http://datamine.mta.info/mta_esi.php?key=" + apikey_str + "&feed_id=21"));

		std::unique_ptr<IFeedService<GtfsFeedParser>> busTUFeedParser = std::make_unique<DynamicBusFeedService>(
			"http://gtfsrt.prod.obanyc.com/tripUpdates?key=");

		std::unique_ptr<IFeedService<GtfsFeedParser>> busVUFeedParser = std::make_unique<DynamicBusFeedService>(
			"http://gtfsrt.prod.obanyc.com/vehiclePositions?key=");

/*		auto rfs = new ReplayFeedService<NYCTFeedParser>("C:/Users/Extreme/CMakeBuilds/9a20cf93-1bae-a730-96f6-0e9e11475965/build/x86-Debug", "gtfs_nycts");
		rfs->jumpTo("gtfs_nycts_1531197614.bin");
		rfs->setMaximumPlaybacks(1);
			
		auto feedService = std::unique_ptr<IFeedService>((IFeedService*)rfs);
*/		

		auto event_holder = std::make_shared<BlockingEventHolder<SubwayTripEvent>>();
		auto bus_event_holder = std::make_shared<BlockingEventHolder<NYCBusTripEvent>>();
		NYCTFeedTracker nyctFeedTracker123456(feedService123456, event_holder);
		NYCTFeedTracker nyctFeedTrackerBDFM(feedServiceBDFM, event_holder);
		NYCTFeedTracker nyctFeedTrackerL(feedServiceL, event_holder);

		NYCBusTracker nycBusTracker(busTUFeedParser, busVUFeedParser, bus_event_holder);

		//auto trips = nyctFeedTracker.getTripsScheduledToArriveAtStop("217S");
		/*auto memleaktest = std::thread([&]() {
			std::this_thread::sleep_for(std::chrono::seconds(60));
			nyctFeedTracker.kill();
		});
		memleaktest.detach();*/

		auto tracker_running123456 = std::thread([&]() {
			nyctFeedTracker123456.run();
		});

		auto tracker_runningBDFM = std::thread([&]() {
			//nyctFeedTrackerBDFM.run();
		});

		auto tracker_runningL = std::thread([&]() {
			//nyctFeedTrackerL.run();
		});

		auto tracker_runningBUS = std::thread([&]() {
			//nycBusTracker.run();
		});

		tracker_running123456.detach();
		tracker_runningBDFM.detach();
		tracker_runningL.detach();
		tracker_runningBUS.detach();
		WSInterface wsi;
		auto subway_ws_interface = NYCTSubwayInterface(&wsi, { &nyctFeedTracker123456, &nyctFeedTrackerBDFM, &nyctFeedTrackerL }, event_holder);

		//auto bus_ws_interface = NYCBusInterface(&wsi, { &nycBusTracker }, bus_event_holder);

		auto ws_listening_thread = std::thread([&] {
			wsi.start("", 7777);
		});

		auto event_handling_thread = std::thread([&] {
			subway_ws_interface.run();
			//bus_ws_interface.run();
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
