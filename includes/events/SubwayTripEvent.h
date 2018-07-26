#pragma once
#include "BaseEvent.h"

#include <string>
#include "subway/SubwayTrackedTrip.h"

namespace nyctlib {
	struct SubwayTripEvent : public BaseEvent {

		enum Category {
			Delay,
			StopChange,
			ScheduleChange,
			TrackChange,
			LostStopData, // did not confirm that train arrived or stopped
			TripAssigned,
			TripUnassigned,
			LostTrip, // lost track of a trip
			TripComplete,
		};

		// Should all point to member temporary variables and not need to be freed.
		// The event producer should keep these in memory for as long as needed.
		std::string trip_id;
		SubwayTrackedTrip *initial_tracked_trip;
		Category event_category;
		std::string paramaters;
		NYCTTripUpdate trip_update;
		NYCTVehicleUpdate vehicle_update;

		inline Category getEventCategory() {
			return event_category;
		}

		inline SubwayTrackedTrip* getInitialTrackedTrip() {
			return initial_tracked_trip;
		}

		inline NYCTTripUpdate getTripUpdate() {
			return trip_update;
		}

		inline NYCTVehicleUpdate getVehicleUpdate() {
			return vehicle_update;
		}

		std::string getParamaters() {
			return paramaters;
		}

		virtual EventType getType() override {
			return EventType::SubwayTripEvent;
		}
	};
}