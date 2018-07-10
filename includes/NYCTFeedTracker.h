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

		struct TrackedTrip {
			int cumulative_arrival_delay = 0;
			int cumulative_depature_delay = 0;

			NYCTTripUpdate last_tracked_trip;
			NYCTVehicleUpdate last_tracked_vehicle;

			NYCTTripUpdate old_tracked_trip;

			std::vector<NYCTTripTimeUpdate> initial_trip_schedule;

			std::vector<std::tuple<std::string, long long>> confirmed_stops;
		};

		std::map<std::string /* ATS ID */, TrackedTrip> tracked_trips2;
		std::atomic<bool> active;

		void clearTrackedDataForTrip(std::string tripid);

		void processTripTimeUpdates(std::string tripid, std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &old, std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &current);

		int getStopIndexRelativeToInitialSchedule(std::string trip_id, std::string gtfs_stop_id);
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

		inline NYCTTripUpdate getTrackedTripUpdate(std::string ats_trip_id) {
			if (this->tracked_trips2.find(ats_trip_id) != this->tracked_trips2.end()) {
				return this->tracked_trips2[ats_trip_id].last_tracked_trip;
			}
			throw std::invalid_argument("no such trip id is currently tracked");
		}

		std::vector<NYCTTripUpdate> getTripsScheduledToArriveAtStop(std::string station_id);

		void printTripsScheduledToArriveAtStop(std::string station_id);
	};
}