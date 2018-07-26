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
	json11::Json NYCTSubwayInterface::buildTripScheduleUpdate(SubwayTripEvent &e) {
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
				{ "DepatureTime", (int)s->depature_time },
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
					{ "DepatureTime", (int)s->depature_time },
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