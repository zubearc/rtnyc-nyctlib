#include "GtfsFeedParser.h"

#include <istream>
#include <fstream>

#include "TimeUtil.h"

namespace nyctlib {

	GtfsFeedParser::GtfsFeedParser() {

	}


	bool GtfsFeedParser::loadTrip(const transit_realtime::TripDescriptor &trip) {
		std::string tripId = trip.trip_id();
		std::string routeId = trip.route_id();
		std::string startDate = trip.start_date();
		auto startTime = trip.start_time();
		// schedule relationship not read

		printf("tripId=%s, routeId=%s, startDate=%s\n",
			tripId.c_str(), routeId.c_str(), startDate.c_str());

		return true;
	}

	bool GtfsFeedParser::loadStopTimeUpdate(const transit_realtime::TripUpdate_StopTimeUpdate &stoptimeupdate) {

		return false;
	}

	// INTERNAL PARSING + LOADING
	bool GtfsFeedParser::parseTripUpdate(const transit_realtime::TripUpdate &tripupdate) {
		long long timestamp = tripupdate.timestamp();
		auto trip = tripupdate.trip();
		auto stoptimeupdates = tripupdate.stop_time_update();

		this->loadTrip(trip);

		for (auto stoptimeupdate : stoptimeupdates) {
			long long arrival_time = 0;
			int arrival_delay = 0;
			int arrival_uncertanty = 0;

			long long depature_time = 0;
			int depature_delay = 0;
			int depature_uncertanty = 0;

			if (stoptimeupdate.has_arrival()) {
				auto arrival = stoptimeupdate.arrival();
				arrival_delay = arrival.delay();
				arrival_time = arrival.time();
				arrival_uncertanty = arrival.uncertainty();
			}

			if (stoptimeupdate.has_departure()) {
				auto depature = stoptimeupdate.departure();
				depature_time = depature.time();
				depature_delay = depature.delay();
				depature_uncertanty = depature.uncertainty();
			}

			std::string stop_id = stoptimeupdate.stop_id();

			printf("   -- arriving to '%s' at %lld, departing at %lld (%lld sec wait)\n",
				stop_id.c_str(), arrival_time, depature_time, depature_time - arrival_time);

		}

		return false;
	}

	bool GtfsFeedParser::parseVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition) {
		int currentStopIndex = vehicleposition.current_stop_sequence();
		auto currentStopProgress = vehicleposition.current_status();
		long long timestamp = vehicleposition.timestamp();
		std::string stopId = vehicleposition.stop_id();

		this->loadTrip(vehicleposition.trip());

		if (currentStopProgress == transit_realtime::VehiclePosition_VehicleStopStatus_INCOMING_AT) {
			printf(" - is approaching stop #%d, %s (last updated %lld)\n", currentStopIndex, stopId.c_str(), timestamp);
		} else if (currentStopProgress == transit_realtime::VehiclePosition_VehicleStopStatus_IN_TRANSIT_TO) {
			printf(" - is enroute to stop #%d, %s (last updated %lld)\n", currentStopIndex, stopId.c_str(), timestamp);
		} else if (currentStopProgress == transit_realtime::VehiclePosition_VehicleStopStatus_STOPPED_AT) {
			printf(" - is at stop #%d, %s (last updated %lld)\n", currentStopIndex, stopId.c_str(), timestamp);
		} else {
			throw std::exception("unknown stop progress id");
		}

		return true;
;	}

	bool GtfsFeedParser::readHeader(const transit_realtime::FeedHeader &header) noexcept {
		std::string realtime_version = header.gtfs_realtime_version();
		long long timestamp = header.timestamp();

		printf("GtfsFeedParser: Read header with version '%s' at '%s' (%lld)\n", realtime_version.c_str(), time_str_from_unixtime(timestamp).c_str(), timestamp);

		return true;
	}

	bool GtfsFeedParser::readBody(const transit_realtime::FeedEntity &entity) noexcept {
		std::string entity_id = entity.id();

		if (entity.has_trip_update()) {
			this->parseTripUpdate(entity.trip_update());
		}

		if (entity.has_vehicle()) {
			this->parseVehicleUpdate(entity.vehicle());
		}

		return false;
	}
	bool GtfsFeedParser::loadFile(std::string filename) noexcept {
		std::fstream input(filename, std::ios::in | std::ios::binary);
		transit_realtime::FeedMessage feedMessage;
		if (!input) {
			std::cout << filename << ": File not found." << std::endl;
		} else if (!feedMessage.ParseFromIstream(&input)) {
			std::cerr << "Failed to parse protocol buffer, is it valid?" << std::endl;
			return false;
		}

		return this->readFeedMessage(feedMessage);
	}
	bool GtfsFeedParser::loadBuffer(std::string &data) noexcept {
		transit_realtime::FeedMessage feedMessage;

		if (!feedMessage.ParseFromString(data)) {
			fprintf(stderr, "Failed to read protocol buffer");
			return false;
		}

		return readFeedMessage(feedMessage);
	}
	
	bool GtfsFeedParser::readFeedMessage(const transit_realtime::FeedMessage &message) {

		if (!message.IsInitialized())
			return false;

		this->readHeader(message.header());

		for (auto feedentity : message.entity()) {
			this->readBody(feedentity);
		}
		

		return false;
	}
}