#pragma once

#include "NYCTFeedService.h"
#include "Globals.h"

namespace nyctlib {
	class NYCTFeedTracker {
		std::unique_ptr<IFeedService> feed;

		int tracked_ticker = 0;

		long long last_update_time;

		std::map<std::string /* ATS ID */, NYCTTripUpdate> tracked_trips;
	public:
#ifndef _EMSCRIPTEN
		NYCTFeedTracker() : feed(std::make_unique<NYCTFeedService>()) {}
#endif

		NYCTFeedTracker(std::unique_ptr<IFeedService> &&feed) : feed(std::move(feed)) {};

		void update();

		inline IFeedService* getFeedService() {
			return feed.get();
		}

		std::vector<NYCTTripUpdate> getTripsScheduledToArriveAtStop(std::string station_id);

		void printTripsScheduledToArriveAtStop(std::string station_id);
	};
}