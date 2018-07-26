#pragma once

#include <thread>
#include <atomic>
#include "NYCTFeedService.h"
#include "Globals.h"
#include "subway/SubwayTrackedTrip.h"
#ifndef NO_INTERFACES
#include "events/EventHolder.h"
#endif
#include "events/SubwayTripEvent.h"


namespace nyctlib {
	class NYCTFeedTracker {
		std::unique_ptr<IFeedService> feed;

		int tracked_ticker = 0;

		long long last_update_time;


		std::map<std::string /* ATS ID */, SubwayTrackedTrip> tracked_trips2;
		std::atomic<bool> active;

#ifndef NO_INTERFACES
		std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> eventHolder;
#endif

		void clearTrackedDataForTrip(std::string tripid);

		void processTripTimeUpdates(std::string tripid, std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &old, std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &current, const NYCTTripUpdate *new_tu = nullptr);

		int getStopIndexRelativeToInitialSchedule(std::string trip_id, std::string gtfs_stop_id);
	
		std::vector<SubwayTripEvent> _event_queue;

		inline void queueEvent(SubwayTripEvent &event) {
#ifndef NO_INTERFACES
			_event_queue.push_back(event);
#endif
		}

		inline void flushEvents() {
#ifndef NO_INTERFACES
			if (this->eventHolder) {
				this->eventHolder->queue.enqueue_bulk(_event_queue.begin(), _event_queue.size());
			}
#endif
			_event_queue.clear();

		}
	public:
#ifndef _EMSCRIPTEN
		NYCTFeedTracker() : feed(std::make_unique<NYCTFeedService>()) {}
#endif

		NYCTFeedTracker(std::unique_ptr<IFeedService> &&feed) : feed(std::move(feed)) {};

		NYCTFeedTracker(std::unique_ptr<IFeedService> &&feed, std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> event_holder)
			: feed(std::move(feed)), eventHolder(event_holder) {};

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

		inline SubwayTrackedTrip& getTrackedTrip(std::string ats_trip_id) {
			if (this->tracked_trips2.find(ats_trip_id) != this->tracked_trips2.end()) {
				return this->tracked_trips2[ats_trip_id];
			}
			throw std::invalid_argument("no such trip id is currently tracked");
		}

		std::vector<NYCTTripUpdate> getTripsScheduledToArriveAtStop(std::string station_id);

		void printTripsScheduledToArriveAtStop(std::string station_id);
	};
}