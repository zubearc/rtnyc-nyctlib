#include "../includes/WASMSubway.h"

#include <stdio.h>

#include "NYCTFeedTracker.h"

#include "EmscriptenNYCTFeedService.h"

using namespace nyctlib;

EXPORTABLE void nyctlib_init() {
	printf("Hello from WASMSubway.cpp!\n");
}

EXPORTABLE PNYCTFeedTracker nyctlib_NYCTFeedTracker_create() {
	//NYCTFeedService feedService;
	auto feedService = std::unique_ptr<IFeedService>(new EmscriptenNYCTFeedService());
	auto tracker = new NYCTFeedTracker(std::move(feedService));
	return tracker;
}

EXPORTABLE bool nyctlib_NYCTFeedTracker_loadbuffer(PNYCTFeedTracker tracker, const char *buffer) {
	auto trip_update = (NYCTFeedTracker*)tracker;
	//printf("Got: %s\n", buffer);
	return true;
}

EXPORTABLE bool nyctlib_NYCTFeedTracker_printTripsScheduledToArriveAtStop(PNYCTFeedTracker tracker, const char *station_id) {
	auto trip_update = (NYCTFeedTracker*)tracker;
	printf("Checking stop %s\n", station_id);
	trip_update->printTripsScheduledToArriveAtStop("217S");
	return true;
}

EXPORTABLE void nyctlib_NYCTFeedTracker_destroy(PNYCTFeedTracker tracker) {
	delete (NYCTFeedTracker*)tracker;
}
