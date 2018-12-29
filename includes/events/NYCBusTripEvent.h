#pragma once
#include "BaseEvent.h"

#include <string>
#include "busses/NYCBusTrackedTrip.h"

namespace nyctlib {
	struct NYCBusTripEvent : public BaseEvent {

		enum Category {
			Delay,
			StopChange,
			PositionChange,
			ScheduleChange,
			LostTrip, // lost track of a trip
			TripComplete,
		};

		// Should all point to member temporary variables and not need to be freed.
		// The event producer should keep these in memory for as long as needed.
		std::string trip_id;
		NYCBusTrackedTrip *initial_tracked_trip;
		Category event_category;
		std::string paramaters;
		GtfsTripUpdate trip_update;
		GtfsVehicleUpdate vehicle_update;

		inline Category getEventCategory() {
			return event_category;
		}

		inline NYCBusTrackedTrip* getInitialTrackedTrip() {
			return initial_tracked_trip;
		}

		inline GtfsTripUpdate getTripUpdate() {
			return trip_update;
		}

		inline GtfsVehicleUpdate getVehicleUpdate() {
			return vehicle_update;
		}

		std::string getParamaters() {
			return paramaters;
		}

		virtual EventType getType() override {
			return EventType::BusTripEvent;
		}
	};
}