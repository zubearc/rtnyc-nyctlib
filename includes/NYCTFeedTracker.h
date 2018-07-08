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
		std::map<std::string, /* Vehicle Data */ NYCTVehicleUpdate> tracked_vehicles;
		std::map<std::string /* ATS ID */, int /* Cumulative Delays */> tracked_trips_arrival_delays;
		std::map<std::string /* ATS ID */, int /* Cumulative Delays */> tracked_trips_depature_delays;

		std::atomic<bool> active;

		void clearTrackedDataForTrip(std::string tripid);
	public:
#ifndef _EMSCRIPTEN
		NYCTFeedTracker() : feed(std::make_unique<NYCTFeedService>()) {}
#endif

		NYCTFeedTracker(std::unique_ptr<IFeedService> &&feed) : feed(std::move(feed)) {};

		bool update();

		void run();

		inline void kill() {
			active = false;
		}

		inline IFeedService* getFeedService() {
			return feed.get();
		}

		std::vector<NYCTTripUpdate> getTripsScheduledToArriveAtStop(std::string station_id);

		void printTripsScheduledToArriveAtStop(std::string station_id);
	};
}