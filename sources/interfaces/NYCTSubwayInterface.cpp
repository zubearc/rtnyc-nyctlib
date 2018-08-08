#include <future>
#include "interfaces/NYCTSubwayInterface.h"

using namespace json11;

namespace nyctlib {
	NYCTSubwayInterface::NYCTSubwayInterface(WSInterface *ws_interface, NYCTFeedTracker *tracker,
		std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> event_holder) {
		
		ws_interface->setRecieveHandler([&](WSInterface::Client client, Json &data) {
			printf("recieveHandler(): '%d': Got '%s'\n", client, data.dump().c_str());
		});

		ws_interface->setCommandHandler("subway", [&](WSInterface::Client client, std::string command, std::string arguments) {
			//this->processRequest(client, command, arguments);
			std::async(&NYCTSubwayInterface::processRequest, this, client, command, arguments);
		});

		this->holder = event_holder;
		this->wsInterface = ws_interface;
		this->tracker = tracker;
	}

	void NYCTSubwayInterface::processRequest(WSInterface::Client client, std::string request, std::string paramaters) {
		if (request == "get-trip-update") {
			pLatestTripUpdateRequest(client, paramaters);
		} else {
			this->wsInterface->respondError(client, "no such command");
		}
	}

	json11::Json NYCTSubwayInterface::jBuildStopMessageUpdate(SubwayTripEvent &e) {
		auto vu = &e.vehicle_update;
		auto currently_relevant_stop_id = vu->stop_id;
		auto tracked = e.initial_tracked_trip;
		auto stu = tracked->getStopScheduled(currently_relevant_stop_id);
		int current_arrival_time = INT_MIN;
		int current_depature_time = INT_MIN;
		if (stu) {
			current_arrival_time = stu->arrival_time;
			current_depature_time = stu->depature_time;
		}

		json11::Json::object json{
			{ "timestamp", (int)tracked->last_update_time },
			{ "TripID", e.trip_id },
			{ "CurrentStatus", (int)vu->stop_progress },
			{ "CumulativeArrivalDelay", tracked->cumulative_arrival_delay },
			{ "CumulativeDepartureDelay", tracked->cumulative_depature_delay },
			{ "OnSchedule", tracked->updated_trip_schedules.size() == 0 ? true : false }
		};

		if (vu->stop_progress == GtfsVehicleProgress::AtStation) {
			auto next_stop = e.initial_tracked_trip->getNextStopScheduled(vu->stop_id);
			json["CurrentStopID"] = vu->stop_id;
			json["CurrentStopDepartingOn"] = current_depature_time;

			if (next_stop) {
				json["NextScheduledStop"] = next_stop->stop_id;
				json["NextStopArrivingOn"] = (int)next_stop->arrival_time;
				json["NextStopDepartingOn"] = (int)next_stop->depature_time;
			}
		} else {
			NYCTTripTimeUpdate *last_stop = e.initial_tracked_trip->getPreviousStopScheduled(vu->stop_id);

			if (last_stop != nullptr) {
				if (last_stop->stop_id.size() > 0) {
					json["LastStopID"] = last_stop->stop_id;
				}
			}

			json["NextScheduledStop"] = vu->stop_id;
			json["NextStopArrivingOn"] = current_arrival_time;
			json["NextStopDepartingOn"] = current_depature_time;
		}

		json11::Json j(json);
		return j;
	}
	json11::Json NYCTSubwayInterface::jBuildTripScheduleUpdate(SubwayTripEvent &e) {
		auto tu = e.trip_update;
		NYCTTrip *trip = (NYCTTrip*)tu.trip.get();

		Json::object json;
		json["trip"] = Json::object{
			{ "GTFSTripID", trip->trip_id },
			{ "StartDate", trip->start_date },
			{ "RouteID", trip->route_id },
			{ "TripID", trip->nyct_train_id },
			{ "Assigned", trip->nyct_is_assigned },
			{ "Direction", trip->nyct_direction }
		};
		json["timestamp"] = (int)tu.timestamp;
		Json::array schedule;
		for (auto &stu : tu.stop_time_updates) {
			auto s = (NYCTTripTimeUpdate*)stu.get();
			schedule.push_back(Json::object{
				{ "ScheduledTrack", s->scheduled_track },
				{ "ActualTrack", s->actual_track },
				{ "ArrivalTime", (int)s->arrival_time },
				{ "DepartureTime", (int)s->depature_time },
				{ "StopID", s->stop_id }
				});
		}
		json["schedule"] = schedule;

		json11::Json j(json);
		return j;

		/*std::string event_type;
		if (e.event_category == SubwayTripEvent::TripAssigned)
			event_type = "TripAssigned";
		else if (e.event_category == SubwayTripEvent::ScheduleChange)
			event_type = "ScheduleChange";
		else
			event_type = "UnknownTripUpdate";
		wsInterface->broadcast("subway", event_type, data);*/
	}

	void NYCTSubwayInterface::run() {
		running = true;
		while (running) {
			SubwayTripEvent tracked_trip[16];
			auto count = holder->queue.wait_dequeue_bulk(tracked_trip, 16);

			std::vector<std::pair<std::string /* command */, json11::Json>> data;

			for (int i = 0; i < count; i++) {
				auto trip = tracked_trip[i];
				json11::Json *j = nullptr;
				switch (trip.event_category) {
				case SubwayTripEvent::StopChange:
					data.push_back(std::make_pair("StopChange", jBuildStopMessageUpdate(trip)));
					break;
				case SubwayTripEvent::TripAssigned:
					data.push_back(std::make_pair("TripAssigned", jBuildTripScheduleUpdate(trip)));
					break;
				case SubwayTripEvent::ScheduleChange:
					data.push_back(std::make_pair("ScheduleChange", jBuildTripScheduleUpdate(trip)));
					break;
				default:
					printf("Ignoring unknown event\n");
				}
			}
			this->wsInterface->broadcast("subway", data);

			/*if (gf_subscribed_clients.size() > 0) {
			switch (tracked_trip.event_category) {
			case SubwayTripEvent::StopChange:

			}
			}*/

		}

	}
	void NYCTSubwayInterface::pLatestTripUpdateRequest(WSInterface::Client client, std::string trip_id) {
		try {
			auto tu = tracker->getTrackedTripUpdate(trip_id);
			Json::object json;
			NYCTTrip *trip = (NYCTTrip*)tu.trip.get();
			json["trip"] = Json::object{
				{ "GTFSTripID", trip->trip_id },
				{ "StartDate", trip->start_date },
				{ "RouteID", trip->route_id },
				{ "TripID", trip->nyct_train_id },
				{ "Assigned", trip->nyct_is_assigned },
				{ "Direction", trip->nyct_direction }
			};
			json["timestamp"] = (int)tu.timestamp;
			Json::array schedule;
			for (auto &stu : tu.stop_time_updates) {
				auto s = (NYCTTripTimeUpdate*)stu.get();
				schedule.push_back(Json::object{
					{ "ScheduledTrack", s->scheduled_track },
					{ "ActualTrack", s->actual_track },
					{ "ArrivalTime", (int)s->arrival_time },
					{ "DepartureTime", (int)s->depature_time },
					{ "StopID", s->stop_id }
				});
			}
			json["schedule"] = schedule;

			Json j(json);
			this->wsInterface->respondCommand(client, "subway", "get-trip-update", j);
		} catch (std::invalid_argument &err) {
			this->wsInterface->respondError(client, std::string("Trip error: ") + err.what());
		}
	}
}