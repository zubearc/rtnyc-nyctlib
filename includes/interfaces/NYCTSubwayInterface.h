#pragma once

#include <atomic>
#include "WSInterface.h"
#include "subway/NYCTFeedTracker.h"
#include "events/SubwayTripEvent.h"
#include "events/EventHolder.h"

namespace nyctlib {
	class NYCTSubwayInterface {
	public:
		enum SupportedOperations {
			GetTripStatus,
			GetTripsForRoute,
			GetTripsForStop
		};
	private:

		std::vector<WSInterface::Client> gf_subscribed_clients;

		std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> holder;

		WSInterface *wsInterface;
		NYCTFeedTracker *tracker;
	public:
		volatile bool running;

		NYCTSubwayInterface(WSInterface *ws_interface, NYCTFeedTracker *tracker, 
			std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> event_holder);

		void processRequest(WSInterface::Client client, std::string request, std::string paramaters);

		void respond(WSInterface::Client client, std::string data) {
			if (client != nullptr) {
				//this->wsInterface->respondCommand(client, "subway", "");
			}
		}

		json11::Json buildStopMessageUpdate(SubwayTripEvent &e) {
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
				{ "TripID", e.trip_id },
				{ "CurrentStatus", (int)vu->stop_progress },
				{ "CumulativeArrivalDelay", tracked->cumulative_arrival_delay},
				{ "CumulativeDepatureDelay", tracked->cumulative_depature_delay },
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
						assert(last_stop->stop_id.size() > 0);
						json["LastStopID"] = last_stop->stop_id;
					}
				}
				
				json["NextScheduledStop"] = vu->stop_id;
				json["NextStopArrivingOn"] = current_arrival_time;
				json["NextStopDepartingOn"] = current_depature_time;
			}

			json11::Json j(json);
			return j;
			//data.push_back(j);
			//wsInterface->broadcast("subway", "StopChange", data);
			// TripID
		}

		json11::Json buildTripScheduleUpdate(SubwayTripEvent &e);

		void run() {
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
						data.push_back(std::make_pair("StopChange", buildStopMessageUpdate(trip)));
						break;
					case SubwayTripEvent::TripAssigned:
						data.push_back(std::make_pair("TripAssigned", buildTripScheduleUpdate(trip)));
						break;
					case SubwayTripEvent::ScheduleChange:
						data.push_back(std::make_pair("ScheduleChange", buildTripScheduleUpdate(trip)));
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

		void pGeneralFeedSubscribeRequest(WSInterface::Client client) {
			this->gf_subscribed_clients.push_back(client);
		}

		void pLatestTripUpdateRequest(WSInterface::Client client, std::string trip_id);
	};
}