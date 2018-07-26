#include "gtfs/NYCTFeedParser.h"

namespace nyctlib {

	bool NYCTFeedParser::parseStopTimeUpdate(const transit_realtime::TripUpdate_StopTimeUpdate &stoptimeupdate, NYCTTripTimeUpdate &out) {
		bool r = GtfsFeedParser::parseStopTimeUpdate(stoptimeupdate, out);

		if (!r)
			return r;

		auto nyctStopTimeUpdate = stoptimeupdate.GetExtension(nyct_stop_time_update);

		out.scheduled_track = nyctStopTimeUpdate.scheduled_track();
		out.actual_track = nyctStopTimeUpdate.actual_track();

		return true;
	}

	bool NYCTFeedParser::loadTrip(const transit_realtime::TripDescriptor &trip, NYCTTrip &out) {
		auto r = GtfsFeedParser::loadTrip(trip, out);
		if (!r)
			return r;

		auto nyctTrip = trip.GetExtension(nyct_trip_descriptor);
		out.nyct_train_id = nyctTrip.train_id();
		out.nyct_direction = (int)nyctTrip.direction();
		out.nyct_is_assigned = nyctTrip.is_assigned();

		return true;
	}
	bool NYCTFeedParser::readHeader(const transit_realtime::FeedHeader &header) noexcept {
		// first try to read GTFS data
		auto r = GtfsFeedParser::readHeader(header);
		if (!r)
			return r;

		auto nyctHeader = header.GetExtension(nyct_feed_header);
		auto nyctFeedVersion = nyctHeader.nyct_subway_version();

		printf("NYCTFeedParser: Loaded NYCT feed extensions v%s\n", nyctFeedVersion.c_str());
		//todo: read trip_replacement_data

		return true;
	}

	bool NYCTFeedParser::loadTripUpdate(const transit_realtime::TripUpdate &tripupdate, NYCTTripUpdate &out) {
		GtfsFeedParser::parseTripUpdate(tripupdate, out);
		
		auto out_trip = std::make_shared<NYCTTrip>();
		loadTrip(tripupdate.trip(), *out_trip.get());
		out.trip = std::move(out_trip);

		auto stoptimeupdates = tripupdate.stop_time_update();

		for (auto stoptimeupdate : stoptimeupdates) {
			auto u = std::make_shared<NYCTTripTimeUpdate>();
			parseStopTimeUpdate(stoptimeupdate, *u.get());
			out.stop_time_updates.push_back(std::move(u));
		}

		return true;
	}

	bool NYCTFeedParser::loadVehicleUpdate(const transit_realtime::VehiclePosition &vehicleposition, NYCTVehicleUpdate &out) {
		bool r = GtfsFeedParser::parseVehicleUpdate(vehicleposition, out);

		if (!r) {
			//DEBUGASSERT
			return r;
		}

		auto out_trip = std::make_shared<NYCTTrip>();
		this->loadTrip(vehicleposition.trip(), *out_trip.get());
		out.trip = std::move(out_trip);
		return true;
	}

	bool NYCTFeedParser::readBody(const transit_realtime::FeedEntity &entity) noexcept {
		std::string entity_id = entity.id();

		if (entity.has_trip_update()) {
			auto trip_update = std::make_shared<NYCTTripUpdate>();
			if (!this->loadTripUpdate(entity.trip_update(), *trip_update.get())) {
				fprintf(stderr, "FAILED TO PARSE TRIP UPDATE for entity '%s'\n", entity_id.c_str());
				return false;
			}
			this->addTripUpdate(std::move(trip_update));
		}

		if (entity.has_vehicle()) {
			auto vech_update = std::make_shared<NYCTVehicleUpdate>();
			if (!this->loadVehicleUpdate(entity.vehicle(), *vech_update.get())) {
				fprintf(stderr, "FAILED TO PARSE VEHICLE UPDATE for entity '%s'\n", entity_id.c_str());
				return false;
			}
			this->addVehicleUpdate(std::move(vech_update));
		}

		return true;
	}

	void NYCTFeedParser::dumpOut() {
		forEachVehicleUpdate([](NYCTVehicleUpdate *v) {
			PRINTDBG("Found vehicle at stop #%d (stop id %s) at %lld\n", v->current_stop_index, v->stop_id.c_str(), v->timestamp);
			auto trip_wrapped = v->trip;
			NYCTTrip *trip = (NYCTTrip*)trip_wrapped.get();
			PRINTDBG("\t route '%s', trip id '%s', at %s is ", trip->route_id.c_str(), trip->trip_id.c_str(), trip->start_date.c_str());
			if (v->stop_progress == GtfsVehicleProgress::ApproachingStation)
				PRINTDBG("approaching -- ");
			if (v->stop_progress == GtfsVehicleProgress::EnrouteToStation)
				PRINTDBG("enroute to station -- ");
			if (v->stop_progress == GtfsVehicleProgress::AtStation)
				PRINTDBG("at station -- ");

			PRINTDBG("ATS ID '%s' isAssigned: %d Direction: %d\n", trip->nyct_train_id.c_str(), trip->nyct_is_assigned, trip->nyct_direction);
		});

		forEachTripUpdate([](const NYCTTripUpdate &tu) {
			auto t = &tu;
			auto trip_wrapped = t->trip;
			NYCTTrip *trip = (NYCTTrip*)trip_wrapped.get();
			PRINTDBG("Trip route '%s', trip id '%s', at %s -- ", trip->route_id.c_str(), trip->trip_id.c_str(), trip->start_date.c_str());
			
			PRINTDBG("ATS ID '%s' isAssigned: %d Direction: %d\n", trip->nyct_train_id.c_str(), trip->nyct_is_assigned, trip->nyct_direction);
		
			for (auto s : t->stop_time_updates) {
				NYCTTripTimeUpdate *u = (NYCTTripTimeUpdate*)s.get();
				PRINTDBG("Expected to arrive at stop '%s' at '%lld', expected to depart at '%lld' -- Expected/Actual Arrival/Depature Tracks: %s/%s\n", 
					u->stop_id.c_str(), u->arrival_time, u->depature_time, u->scheduled_track.c_str(), u->actual_track.c_str());
			}
		});
	}

	/*bool NYCTFeedParser::readFeedMessage(const transit_realtime::FeedMessage &message) {
		
		//NyctFeedHeader nyctFeedHeader = 
		auto header = message.header();
		auto nyct_header = header.GetExtension(nyct_feed_header);
		
		

		return false;
	}*/
}