#pragma once

#include <string>
#include "gtfs-realtime.pb.h"

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
		GtfsTrip trip;
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
		GtfsTrip trip;
		std::vector<GtfsTripTimeUpdate> stop_time_updates;
	};

	class GtfsFeedParser {

		std::string _gtfs_version;

		std::vector<GtfsTripUpdate> trip_updates;
		std::vector<GtfsVehicleUpdate> vehicle_updates;
	private:

		virtual bool loadTrip(const transit_realtime::TripDescriptor &trip, GtfsTrip &out);
		virtual bool loadStopTimeUpdate(const transit_realtime::TripUpdate_StopTimeUpdate &stoptimeupdate);

		virtual bool parseTripUpdate(const transit_realtime::TripUpdate &tripupdate, GtfsTripUpdate &out);
		virtual bool parseVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition, GtfsVehicleUpdate &out);

		virtual bool readHeader(const transit_realtime::FeedHeader &header) noexcept;

		virtual bool readBody(const transit_realtime::FeedEntity &entity) noexcept;

		bool initialized;

	public:
		GtfsFeedParser();
		
		virtual bool loadFile(std::string filename) noexcept;

		virtual bool loadBuffer(std::string &data) noexcept;
		//virtual bool loadBuffer(const char *buffer, int length) noexcept;

		virtual bool readFeedMessage(const transit_realtime::FeedMessage &message);

		void getFeedVersion();

		virtual ~GtfsFeedParser() = default;
	};
}