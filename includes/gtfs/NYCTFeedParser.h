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
	};

	struct NYCTTripUpdate : GtfsTripUpdate {
		NYCTTrip nyct_trip;
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