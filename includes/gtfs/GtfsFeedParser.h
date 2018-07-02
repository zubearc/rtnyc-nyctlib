#pragma once

#include <functional>
#include <string>
#include "gtfs-realtime.pb.h"

//#define PRINTDBG printf
#define PRINTDBG

namespace nyctlib {

	enum class GtfsVehicleProgress {
		AtStation,
		ApproachingStation,
		EnrouteToStation
	};

	struct GtfsTrip {
		std::string trip_id;
		std::string start_date;
		std::string route_id;
	};

	struct GtfsVehicleUpdate {
		long long timestamp = NULL;
		int current_stop_index = -1;
		std::shared_ptr<GtfsTrip> trip;
		GtfsVehicleProgress stop_progress;
		std::string stop_id;
	};

	struct GtfsTripTimeUpdate {
		long long arrival_time = NULL;
		long long depature_time = NULL;
		std::string stop_id;
	};

	struct GtfsTripUpdate {
		long long timestamp = NULL;
		std::shared_ptr<GtfsTrip> trip;
		std::vector<std::shared_ptr<GtfsTripTimeUpdate>> stop_time_updates;
	};

	class GtfsFeedParser {

		std::string _gtfs_version;

		std::vector<std::shared_ptr<GtfsTripUpdate>> trip_updates;
		std::vector<std::shared_ptr<GtfsVehicleUpdate>> vehicle_updates;
	private:
		inline void addTripUpdate(GtfsTripUpdate *tripupdate) {
			trip_updates.push_back(std::shared_ptr<GtfsTripUpdate>(tripupdate));
		}

		inline void addVehicleUpdate(GtfsVehicleUpdate *vehicleupdate) {
			vehicle_updates.push_back(std::shared_ptr<GtfsVehicleUpdate>(vehicleupdate));
		}

		inline void addTripUpdate(std::shared_ptr<GtfsTripUpdate> &&tripupdate) {
			trip_updates.push_back(std::move(tripupdate));
		}

		inline void addVehicleUpdate(std::shared_ptr<GtfsVehicleUpdate> &&vehicleupdate) {
			vehicle_updates.push_back(std::move(vehicleupdate));
		}
	protected:
		virtual bool loadTrip(const transit_realtime::TripDescriptor &trip, GtfsTrip &out);
		virtual bool parseStopTimeUpdate(const transit_realtime::TripUpdate_StopTimeUpdate &stoptimeupdate, GtfsTripTimeUpdate &timeupdate);

		virtual bool parseTripUpdate(const transit_realtime::TripUpdate &tripupdate, GtfsTripUpdate &out);
		virtual bool parseVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition, GtfsVehicleUpdate &out);

		virtual bool loadTripUpdate(const transit_realtime::TripUpdate &tripupdate, GtfsTripUpdate &out);
		virtual bool loadVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition, GtfsVehicleUpdate &out);


		virtual bool readHeader(const transit_realtime::FeedHeader &header) noexcept;

		virtual bool readBody(const transit_realtime::FeedEntity &entity) noexcept;

		bool initialized;

	public:
		GtfsFeedParser();
		
		virtual bool loadFile(std::string filename) noexcept;

		virtual bool loadBuffer(std::string &data) noexcept;

		virtual bool loadBuffer(const char *buffer, int length) noexcept;

		virtual bool readFeedMessage(const transit_realtime::FeedMessage &message);

		virtual void getTripUpdates(std::vector<std::shared_ptr<GtfsTripUpdate>> &v) {
			v = trip_updates;
		}

		virtual void getVehicleUpdates(std::vector<std::shared_ptr<GtfsVehicleUpdate>> &v) {
			v = vehicle_updates;
		}

		virtual void forEachTripUpdate(const std::function<void(GtfsTripUpdate*)> &&l) {
			for (auto i : trip_updates) {
				l(i.get());
			}
		}

		virtual void forEachVehicleUpdate(const std::function<void(GtfsVehicleUpdate*)> &&l) {
			for (auto i : vehicle_updates) {
				l(i.get());
			}
		}

		void getFeedVersion();

		virtual void dumpOut();

		virtual ~GtfsFeedParser() = default;
	};
}
