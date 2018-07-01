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

		std::string trip_id;
		std::string trip_start_date;
		std::string trip_route_id;
		std::string trip_nyct_train_id;
		bool		trip_nyct_is_assigned;
		std::string trip_nyct_direction;

		// ...
	};

	// Container to pointers
	struct CNYCTSubwayTripUpdate {
		void *_NYCTTripUpdate;
	};

	// Return value pointer type mappings to classes
	typedef void* PNYCTFeedTracker;

	EXPORTABLE void nyctlib_init();

	EXPORTABLE PNYCTFeedTracker nyctlib_NYCTFeedService_create();

	EXPORTABLE bool nyctlib_NYCTFeedService_loadbuffer(PNYCTFeedTracker tracker, const char *buffer);

	EXPORTABLE void nyctlib_NYCTFeedService_destroy(PNYCTFeedTracker tracker);

#ifdef __cplusplus
}
#endif