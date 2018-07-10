#pragma once

#include "GtfsFeedParser.h"

#include "nyct-subway.pb.h"

namespace nyctlib {
	struct NYCTTrip : GtfsTrip {
		std::string nyct_train_id;
		bool nyct_is_assigned;
		std::string nyct_direction;
	};

	struct NYCTTripTimeUpdate : GtfsTripTimeUpdate {
		std::string scheduled_track;
		std::string actual_track;
	};

	// Root Messages

	struct NYCTVehicleUpdate : GtfsVehicleUpdate {
		NYCTTrip nyct_trip;
		inline operator bool() const {
			if (!this->trip)
				return false;
			return true;
		}
	};

	struct NYCTTripUpdate : GtfsTripUpdate {
		inline bool operator==(const NYCTTripUpdate& r) {
			for (int i = 0; i < this->stop_time_updates.size(); i++) {
				auto lT = (NYCTTripTimeUpdate*)this->stop_time_updates[i].get();
				auto rT = (NYCTTripTimeUpdate*)r.stop_time_updates[i].get();
				if (!(lT->actual_track == rT->actual_track &&
						lT->arrival_time == rT->arrival_time &&
						lT->depature_time == rT->depature_time &&
						lT->scheduled_track == rT->scheduled_track &&
						lT->stop_id == rT->stop_id)) {
					return false;
				}
			}
			if (this->trip->trip_id != this->trip->trip_id) {
				return false;
			}
			return true;
		}

		inline operator bool() const {
			if (!this->trip)
				return false;
			return true;
		}
	};
	
	class NYCTFeedParser : public GtfsFeedParser {
	public:
	private:

		std::vector<std::shared_ptr<NYCTTripUpdate>> trip_updates;
		std::vector<std::shared_ptr<NYCTVehicleUpdate>> vehicle_updates;

		inline void addTripUpdate(std::shared_ptr<NYCTTripUpdate> &&tripupdate) {
			trip_updates.push_back(std::move(tripupdate));
		}

		inline void addVehicleUpdate(std::shared_ptr<NYCTVehicleUpdate> &&vehicleupdate) {
			vehicle_updates.push_back(std::move(vehicleupdate));
		}
	public:
		virtual bool parseStopTimeUpdate(const transit_realtime::TripUpdate_StopTimeUpdate &stoptimeupdate, NYCTTripTimeUpdate &timeupdate);

		virtual bool loadTrip(const transit_realtime::TripDescriptor &trip, NYCTTrip &out);
		virtual bool readHeader(const transit_realtime::FeedHeader &header) noexcept;
		virtual bool readBody(const transit_realtime::FeedEntity &entity) noexcept;

		virtual bool loadTripUpdate(const transit_realtime::TripUpdate &tripupdate, NYCTTripUpdate &out);
		virtual bool loadVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition, NYCTVehicleUpdate &out);

		virtual void forEachTripUpdate(const std::function<void(NYCTTripUpdate*)> &&l) {
			for (auto &i : trip_updates) {
				l(i.get());
			}
		}

		virtual void forEachVehicleUpdate(const std::function<void(NYCTVehicleUpdate*)> &&l) {
			for (auto &i : vehicle_updates) {
				l(i.get());
			}
		}

		virtual void dumpOut();
	};
}