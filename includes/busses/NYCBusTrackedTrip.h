#pragma once

#include "gtfs/GtfsFeedParser.h"

namespace nyctlib {
	struct NYCBusTrackedTrip {
		long long last_update_time;

		int delay_seconds;

		GtfsTripUpdate last_tracked_trip;
		GtfsVehicleUpdate last_tracked_vehicle;

		std::vector<GtfsTripTimeUpdate> initial_trip_schedule;
		std::vector<std::vector<GtfsTripTimeUpdate>> updated_trip_schedules;

		std::vector<std::pair<std::string /* stop id */, long long>> confirmed_stops;


		// These are unsafe operations and should *only* be used temporarily while this object is in scope

		inline GtfsTripTimeUpdate* isStopScheduledInitially(std::string stop_id, int offset = 0) {

			for (int i = 0; i < initial_trip_schedule.size(); i++) {
				auto &stop = initial_trip_schedule[i];
				if (stop.stop_id == stop_id) {
					if (offset) {
						auto newoff = (i + offset);
						if (newoff < initial_trip_schedule.size() && newoff >= 0) {
							return &initial_trip_schedule[i + offset];
						} else {
							return nullptr;
						}
					}
					return &stop;
				}
			}

			return nullptr;
		}

		inline GtfsTripTimeUpdate* isStopScheduled(std::string stop_id) {
			for (auto &schedule : updated_trip_schedules) {
				for (auto &stop : schedule) {
					if (stop.stop_id == stop_id) {
						return &stop;
					}
				}
			}
			if (updated_trip_schedules.size() == 0) {
				auto scheduled_initially = isStopScheduledInitially(stop_id);
				if (scheduled_initially != nullptr)
					return scheduled_initially;
			}
			return nullptr;
		}

		inline GtfsTripTimeUpdate* getStopScheduled(std::string stop_id, int indexOffset = 0) {
			NYCTTripTimeUpdate *ttu = nullptr;

			int latest_update_i = (int)updated_trip_schedules.size() - 1;

			if (latest_update_i >= 0) {
				auto &latest_trip_schedule = updated_trip_schedules[latest_update_i];
				for (int i = 0; i < latest_trip_schedule.size(); i++) {
					auto &stop = latest_trip_schedule[i];
					if (stop.stop_id == stop_id) {
						if (indexOffset) {
							auto newoff = (i + indexOffset);
							if (newoff < latest_trip_schedule.size() && newoff >= 0) {
								return &latest_trip_schedule[i + indexOffset];
							} else {
								return isStopScheduledInitially(stop_id, indexOffset);
							}
						}
						return &stop;
					}
				}
			} else {
				return isStopScheduledInitially(stop_id, indexOffset);
			}

			return nullptr;
		}

		inline GtfsTripTimeUpdate* getNextStopScheduled(std::string stop_id) {
			return this->getStopScheduled(stop_id, 1);
		}

		inline GtfsTripTimeUpdate* getPreviousStopScheduled(std::string stop_id) {
			return this->getStopScheduled(stop_id, -1);
		}
	};
}