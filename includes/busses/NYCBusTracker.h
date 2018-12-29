#pragma once

// Uses a combination of two different feeds to provide combined data

/*
unfortunately the GTFS feeds are missing quite a bit of data that could be useful to us
like trip origin, destination and user-printable headsigns. the SIRI feeds provide more
of this data. in this library, we focus soley on the GTFS side of things and work under similar
constraints that exist on our NYC subway feed trackers
*/

#include "Globals.h"
#include "IFeedService.h"
#include "NYCBusTrackedTrip.h"
#include "events/NYCBusTripEvent.h"
#include "events/EventHolder.h"
#include <atomic>

namespace nyctlib {
	class NYCBusTracker {
		std::unique_ptr<IFeedService<GtfsFeedParser>> tripUpdateFeed;
		std::unique_ptr<IFeedService<GtfsFeedParser>> vehicleUpdateFeed;

		long long lastUpdatedTime = 0;

		long long vu_feed_updated_time = 0;
		long long tu_feed_updated_time = 0;

		std::map<std::string /* Vehicle ID */, NYCBusTrackedTrip> tracked_trips;

		std::shared_ptr<BlockingEventHolder<NYCBusTripEvent>> eventHolder;

		std::atomic<bool> active;

		std::vector<NYCBusTripEvent> _event_queue;

		void clearTrackedDataForVehicle(std::string vehid);

		inline void queueEvent(NYCBusTripEvent &event) {
#ifndef NO_INTERFACES
			_event_queue.push_back(event);
#endif
		}

	public:
		// vech update feed can be nullptr if not doing any VU checks (e.g. "bearing" may not useful for us here)
		NYCBusTracker(std::unique_ptr<IFeedService<GtfsFeedParser>> &tripUpdateFeed, 
			std::unique_ptr<IFeedService<GtfsFeedParser>> &vehicleUpdateFeed,
			std::shared_ptr<BlockingEventHolder<NYCBusTripEvent>> &event_holder);

		bool update();

		void run();

		inline void flushEvents() {
#ifndef NO_INTERFACES
			if (this->eventHolder) {
				this->eventHolder->queue.enqueue_bulk(_event_queue.begin(), _event_queue.size());
			}
#endif
			_event_queue.clear();
		}

		// Unsafe operation, so please use with caution.
		std::map<std::string /* ATS ID */, NYCBusTrackedTrip>& getTrackedTripsRef() {
			return this->tracked_trips;
		}
	};
	
}