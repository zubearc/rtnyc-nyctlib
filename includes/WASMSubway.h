#pragma once

#include <emscripten.h>

#define EXPORTABLE EMSCRIPTEN_KEEPALIVE

#ifdef __cplusplus
extern "C" {
#endif

	// Struct that will be returned to the user
	// note: need to change std::string to char*
	struct WNYCTSubwayTripUpdate {
		long long	timestamp;
		int			current_stop_index;

		const char* trip_id;
		const char* trip_start_date;
		const char* trip_route_id;
		const char* trip_nyct_train_id;
		bool		trip_nyct_is_assigned;
		const char* trip_nyct_direction;

		// ...
	};

	// Container to pointers
	struct CNYCTSubwayTripUpdate {
		void *_NYCTTripUpdate;
	};

	// Return value pointer type mappings to classes
	typedef void* PNYCTFeedTracker;

	EXPORTABLE void nyctlib_init();

	EXPORTABLE PNYCTFeedTracker nyctlib_NYCTFeedTracker_create();

	EXPORTABLE bool nyctlib_NYCTFeedTracker_loadbuffer(PNYCTFeedTracker tracker, const char *buffer);

	EXPORTABLE bool nyctlib_NYCTFeedTracker_printTripsScheduledToArriveAtStop(PNYCTFeedTracker tracker, const char *station_id);

	EXPORTABLE void nyctlib_NYCTFeedTracker_destroy(PNYCTFeedTracker tracker);

#ifdef __cplusplus
}
#endif