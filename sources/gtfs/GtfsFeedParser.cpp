#include "gtfs/GtfsFeedParser.h"

#include <istream>
#include <fstream>

#include "TimeUtil.h"

namespace nyctlib {

	GtfsFeedParser::GtfsFeedParser() {

	}


	bool GtfsFeedParser::loadTrip(const transit_realtime::TripDescriptor &trip, GtfsTrip &out) {
		out.trip_id = trip.trip_id();
		out.route_id = trip.route_id();
		out.start_date = trip.start_date();
		auto startTime = trip.start_time();
		// schedule relationship not read

		PRINTDBG("tripId=%s, routeId=%s, startDate=%s\n",
			out.trip_id.c_str(), out.route_id.c_str(), out.start_date.c_str());

		return true;
	}

	bool GtfsFeedParser::parseStopTimeUpdate(const transit_realtime::TripUpdate_StopTimeUpdate &stoptimeupdate, GtfsTripTimeUpdate &timeupdate) {
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

		PRINTDBG("   -- arriving to '%s' at %lld, departing at %lld (%lld sec wait)\n",
			stop_id.c_str(), arrival_time, depature_time, depature_time - arrival_time);
		
		timeupdate.arrival_time = arrival_time;
		timeupdate.depature_time = depature_time;
		timeupdate.stop_id = stop_id;
		return true;
	}

	// INTERNAL PARSING + LOADING
	bool GtfsFeedParser::parseTripUpdate(const transit_realtime::TripUpdate &tripupdate, GtfsTripUpdate &out) {
		out.timestamp = tripupdate.timestamp();
		return true;
	}

	bool GtfsFeedParser::parseVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition, GtfsVehicleUpdate &out) {
		out.current_stop_index = vehicleposition.current_stop_sequence();
		auto currentStopProgress = vehicleposition.current_status();
		out.timestamp = vehicleposition.timestamp();
		out.stop_id = vehicleposition.stop_id();

		std::string progressstring;

		if (currentStopProgress == transit_realtime::VehiclePosition_VehicleStopStatus_INCOMING_AT) {
			progressstring = "approaching";
			out.stop_progress = GtfsVehicleProgress::ApproachingStation;
		} else if (currentStopProgress == transit_realtime::VehiclePosition_VehicleStopStatus_IN_TRANSIT_TO) {
			progressstring = "enroute to";
			out.stop_progress = GtfsVehicleProgress::EnrouteToStation;
		} else if (currentStopProgress == transit_realtime::VehiclePosition_VehicleStopStatus_STOPPED_AT) {
			progressstring = "at";
			out.stop_progress = GtfsVehicleProgress::AtStation;
		} else {
			throw std::exception("unknown stop progress id");
		}

		PRINTDBG(" - is %s stop #%d, %s (last updated %lld)\n", progressstring.c_str(), out.current_stop_index, out.stop_id.c_str(), out.timestamp);
	}


	bool GtfsFeedParser::loadTripUpdate(const transit_realtime::TripUpdate &tripupdate, GtfsTripUpdate &out) {
		
		parseTripUpdate(tripupdate, out);

		auto out_trip = std::make_shared<GtfsTrip>();
		loadTrip(tripupdate.trip(), *out_trip.get());
		out.trip = std::move(out_trip);

		auto stoptimeupdates = tripupdate.stop_time_update();

		for (auto stoptimeupdate : stoptimeupdates) {
			auto u = std::make_shared<GtfsTripTimeUpdate>();
			parseStopTimeUpdate(stoptimeupdate, *u.get());
			out.stop_time_updates.push_back(std::move(u));
		}

		return true;
	}

	bool GtfsFeedParser::loadVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition, GtfsVehicleUpdate &out) {

		this->parseVehicleUpdate(vehicleposition, out);

		auto out_trip = std::make_shared<GtfsTrip>();
		this->loadTrip(vehicleposition.trip(), *out_trip.get());
		out.trip = std::move(out_trip);

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
			auto &trip_update = std::make_unique<GtfsTripUpdate>();
			if (!this->loadTripUpdate(entity.trip_update(), *trip_update.get())) {
				fprintf(stderr, "FAILED TO PARSE TRIP UPDATE for entity '%s'\n", entity_id.c_str());
				return false;
			}
			this->addTripUpdate(std::move(trip_update));
		}

		if (entity.has_vehicle()) {
			auto vech_update = std::make_unique<GtfsVehicleUpdate>();
			if (!this->loadVehicleUpdate(entity.vehicle(), *vech_update.get())) {
				fprintf(stderr, "FAILED TO PARSE VEHICLE UPDATE for entity '%s'\n", entity_id.c_str());
				return false;
			}
			this->addVehicleUpdate(std::move(vech_update));
		}

		return true;
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

	void GtfsFeedParser::dumpOut() {

		forEachVehicleUpdate([](GtfsVehicleUpdate *v) {
			PRINTDBG("Found vehicle at stop #%d (stop id %s) at %lld\n", v->current_stop_index, v->stop_id.c_str(), v->timestamp);
			auto &trip = v->trip;
			PRINTDBG("\t route '%s', trip id '%s', at %s is ", trip->route_id.c_str(), trip->trip_id.c_str(), trip->start_date.c_str());
			if (v->stop_progress == GtfsVehicleProgress::ApproachingStation)
				PRINTDBG("approaching\n");
			if (v->stop_progress == GtfsVehicleProgress::EnrouteToStation)
				PRINTDBG("enroute to station\n");
			if (v->stop_progress == GtfsVehicleProgress::AtStation)
				PRINTDBG("at station\n");
		});

		forEachTripUpdate([](GtfsTripUpdate *t) {
			auto &trip = t->trip;
			PRINTDBG("Trip route '%s', trip id '%s', at %s\n", trip->route_id.c_str(), trip->trip_id.c_str(), trip->start_date.c_str());
		});
	}
}