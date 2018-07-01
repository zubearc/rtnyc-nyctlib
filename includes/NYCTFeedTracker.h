#pragma once

#include "NYCTFeedService.h"
#include "Globals.h"

namespace nyctlib {
	class NYCTFeedTracker {
		NYCTFeedService feed;

		int tracked_ticker = 0;

		long long last_update_time;

		std::map<std::string /* ATS ID */, NYCTTripUpdate> tracked_trips;
	public:
#ifndef _EMSCRIPTEN
		NYCTFeedTracker() : feed(NYCTFeedService()) {}
#endif

		NYCTFeedTracker(NYCTFeedService feed) : feed(feed) {};

		void update();

		std::vector<NYCTTripUpdate> getTripsScheduledToArriveAtStop(std::string station_id);

		void printTripsScheduledToArriveAtStop(std::string station_id);
	};
}