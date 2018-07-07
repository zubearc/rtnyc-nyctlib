#include "../includes/WASMSubway.h"

#include <stdio.h>

#include "NYCTFeedTracker.h"

#include "NYCTFeedService.h"

using namespace nyctlib;

EXPORTABLE void nyctlib_init() {
	printf("Hello from WASMSubway.cpp!\n");
}

EXPORTABLE PNYCTFeedTracker nyctlib_NYCTFeedTracker_create() {
	//NYCTFeedService feedService;
	auto feedService = std::unique_ptr<IFeedService>(new NYCTFeedService());
	auto tracker = new NYCTFeedTracker(std::move(feedService));
	return tracker;
}

EXPORTABLE bool nyctlib_NYCTFeedTracker_loadbuffer(PNYCTFeedTracker tracker, const char *buffer) {
	auto trip_update = (NYCTFeedTracker*)tracker;
	//printf("Got: %s\n", buffer);
	return true;
}

EXPORTABLE bool nyctlib_NYCTFeedTracker_updateFromWeb(PNYCTFeedTracker tracker) {
	//tracker->getFeedService()->

	auto onLoad = [](void *arg, void *data, int length) {
		auto trip_update = (NYCTFeedTracker*)arg;
		printf("Got %d bytes of data successfully.\n", length);
		trip_update->getFeedService()->updateFromBuffer((const char*)data, length);
	};

	auto onError = [](void *arg) {
		printf("wget_data failed!\n");
	};

	emscripten_async_wget_data("http://192.168.1.10/api/nyctgtfsproxy/servelatestfeed.php", tracker, onLoad, onError);

	return true;
}

EXPORTABLE bool nyctlib_NYCTFeedTracker_printTripsScheduledToArriveAtStop(PNYCTFeedTracker tracker, const char *station_id) {
	auto feed_tracker = (NYCTFeedTracker*)tracker;
	printf("Checking stop %s\n", station_id);
	feed_tracker->printTripsScheduledToArriveAtStop("217S");
	return true;
}

//typedef void (*tripCallback)(long long timestamp, const char *trip_id, const char *trip_start_time, const char *route_id, const char *trip_nyct_train_id, bool trip_nyct_is_assigned, const char *trip_nyct_direction);

EXPORTABLE bool nyctlib_NYCTFeedTracker_forEachTripScheduledToStopAt(PNYCTFeedTracker tracker, const char *station_id, tripCallback callback) {
	auto feed_tracker = (NYCTFeedTracker*)tracker;
	auto trips_scheduled_to_stop = feed_tracker->getTripsScheduledToArriveAtStop(station_id);

	for (auto trip : trips_scheduled_to_stop) {
		//callback()
		printf("Would callback for trip for route %s\n", trip.trip->trip_id.c_str());
	}
	return true;
}

EXPORTABLE bool nyctlib_NYCTFeedTracker_forEachTimeUpdateForTrip(PNYCTFeedTracker tracker, const char *nyct_trip_id, tripTimeUpdateCallback callback) {
	return true;
}

EXPORTABLE void nyctlib_NYCTFeedTracker_destroy(PNYCTFeedTracker tracker) {
	delete (NYCTFeedTracker*)tracker;
}
