#pragma once

#include "NYCTFeedService.h"
#include "Globals.h"

#include <thread>
#include <atomic>

namespace nyctlib {
	class NYCTFeedTracker {
		std::unique_ptr<IFeedService> feed;

		int tracked_ticker = 0;

		long long last_update_time;

		std::map<std::string /* ATS ID */, NYCTTripUpdate> tracked_trips;
		std::atomic<bool> active;
	public:
#ifndef _EMSCRIPTEN
		NYCTFeedTracker() : feed(std::make_unique<NYCTFeedService>()) {}
#endif

		NYCTFeedTracker(std::unique_ptr<IFeedService> &&feed) : feed(std::move(feed)) {};

		void update();

		void run();

		inline void kill() {
			active = false;
		}

		std::vector<NYCTTripUpdate> getTripsScheduledToArriveAtStop(std::string station_id);

		void printTripsScheduledToArriveAtStop(std::string station_id);
	};
}