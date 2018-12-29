#include "interfaces/NYCBusInterface.h"
#include "rtnyc_protocol_generated.h"
#include <future>

using namespace json11;

namespace nyctlib {
	NYCBusInterface::NYCBusInterface(WSInterface *ws_interface, 
		std::vector<NYCBusTracker*> trackers, 
		std::shared_ptr<BlockingEventHolder<NYCBusTripEvent>> event_holder) {
		ws_interface->setRecieveHandler([&](WSInterface::Client client, Json &data) {
			printf("recieveHandler(): '%d': Got '%s'\n", client, data.dump().c_str());
		});

		ws_interface->setCommandHandler("bus", [&](WSInterface::Client client, std::string command, std::string arguments) {
			//this->processRequest(client, command, arguments);
			std::async(&NYCBusInterface::processRequest, this, client, command, arguments);
		});

		this->holder = event_holder;
		this->wsInterface = ws_interface;
		this->trackers = trackers;
	}

	void NYCBusInterface::processRequest(WSInterface::Client client, std::string request, std::string paramaters) {
		if (request == "get-trip-update") {
			//pLatestTripUpdateRequest(client, paramaters);
		} else if (request == "get-current-trips") {
			pCurrentTripsRequest(client);
		} else {
			this->wsInterface->respondError(client, "no such command");
		}
	}

	json11::Json NYCBusInterface::jBuildStopMessageUpdate(NYCBusTripEvent  &e) {
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
			{ "VehicleID", vu->trip->vehicle_id },
			{ "RouteID", vu->trip->route_id },
			{ "CurrentStatus", (int)vu->stop_progress },
			{ "OnSchedule", tracked->updated_trip_schedules.size() == 0 ? true : false },
			{ "Position", Json::object{
				{ "Latitude", vu->latitude },
				{ "Longitude", vu->longitude },
				{ "Bearing", vu->bearing },
			} }
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
			GtfsTripTimeUpdate *last_stop = e.initial_tracked_trip->getPreviousStopScheduled(vu->stop_id);

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

	json11::Json NYCBusInterface::jBuildTripScheduleUpdate(NYCBusTripEvent &e) {
		auto tu = e.trip_update;
		GtfsTrip *trip = tu.trip.get();

		Json::object json;
		json["trip"] = Json::object{
			{ "GTFSTripID", trip->trip_id },
			{ "StartDate", trip->start_date },
			{ "RouteID", trip->route_id },
		};
		json["timestamp"] = (int)tu.timestamp;
		Json::array schedule;
		for (auto &stu : tu.stop_time_updates) {
			auto s = stu.get();
			schedule.push_back(Json::object{
				{ "ArrivalTime", (int)s->arrival_time },
				{ "DepartureTime", (int)s->depature_time },
				{ "StopID", s->stop_id }
			});
		}
		json["schedule"] = schedule;

		json11::Json j(json);
		return j;
	}

	flatbuffers::Offset<nyc::realtime::NYCBusTrip> buildTrip(flatbuffers::FlatBufferBuilder &builder, GtfsTrip *trip) {
		auto _trip_id = builder.CreateString(trip->trip_id);
		auto _route_id = builder.CreateString(trip->route_id);
		auto _veh_id = builder.CreateString(trip->vehicle_id);
		auto _start_date = builder.CreateString(trip->start_date);

		nyc::realtime::NYCBusTripBuilder trip_buffer(builder);
		trip_buffer.add_gtfs_trip_id(_trip_id);
		trip_buffer.add_route_id(_route_id);
		trip_buffer.add_vehicle_id(_veh_id);
		trip_buffer.add_start_date(_start_date);

		return trip_buffer.Finish();
	}

	void NYCBusInterface::fBuildStopMessageUpdate(NYCBusTripEvent &e, unsigned char *& message, int &message_len) {

		auto vu = &e.vehicle_update;
		auto trip = vu->trip.get();
		auto currently_relevant_stop_id = vu->stop_id;
		auto tracked = e.initial_tracked_trip;
		auto stu = tracked->getStopScheduled(currently_relevant_stop_id);
		int current_arrival_time = INT_MIN;
		int current_depature_time = INT_MIN;

		if (stu) {
			current_arrival_time = stu->arrival_time;
			current_depature_time = stu->depature_time;
		}

		flatbuffers::FlatBufferBuilder builder(1024);

		// We must allocate everything before building the final buffer
		auto _trip_id = builder.CreateString(e.trip_id);
		//flatbuffers::Offset<flatbuffers::String> _current_stop_id;
		flatbuffers::Offset<flatbuffers::String> _next_stop_id;
		//flatbuffers::Offset<flatbuffers::String> _last_stop_id;

		bool _has_next_stop = false;
		bool _has_last_stop = false;
		int _next_stop_arrival_time;
		int _next_stop_departure_time;

		auto trip_buf = buildTrip(builder, trip);

		nyc::realtime::PositionBuilder position_builder(builder);
		position_builder.add_latitude(vu->latitude);
		position_builder.add_longitude(vu->longitude);
		position_builder.add_bearing(vu->bearing);
		auto position = position_builder.Finish();

		nyc::realtime::NYCBusStopUpdateBuilder stop_update(builder);
		stop_update.add_timestamp((int)tracked->last_update_time);
		stop_update.add_trip(trip_buf);

		nyc::realtime::TripStatus ts = *(nyc::realtime::TripStatus*)&vu->stop_progress;
		stop_update.add_current_status(ts);

		stop_update.add_stop_id(_next_stop_id);
		stop_update.add_stop_departing_on(current_depature_time);

		stop_update.add_position(position);

		auto off = stop_update.Finish();
		builder.Finish(off);

		// I don't like the double copy required here, look into avoiding this later
		auto fbuffer = builder.GetBufferPointer();
		int size = builder.GetSize();

		/*std::string sbuf((char*)fbuffer, size);
		printf("\n%d BYTES BEFORE: ", size);
		std::cout << Andromeda::string_to_hex(sbuf) << std::endl;*/

		auto buffer = new unsigned char[size];
		memcpy(buffer, fbuffer, size);

		message = buffer;
		message_len = size;

		// done...

		return;
	}

	void NYCBusInterface::fBuildTripScheduleUpdate(NYCBusTripEvent &e, unsigned char *& message, int &message_len) {
			
		auto tu = e.trip_update;
		GtfsTrip *trip = tu.trip.get();

		flatbuffers::FlatBufferBuilder builder(1024);

		std::vector<flatbuffers::Offset<nyc::realtime::NYCBusSchedule>> schedules;
		e.initial_tracked_trip->last_tracked_vehicle.stop_id;
		for (auto &stu : tu.stop_time_updates) {
			auto s = stu.get();

			auto _stop_id = builder.CreateString(s->stop_id);

			nyc::realtime::NYCBusScheduleBuilder schedule_buffer(builder);
			schedule_buffer.add_departure_time((int)s->arrival_time);
			schedule_buffer.add_stop_id(_stop_id);
			auto off = schedule_buffer.Finish();
			schedules.push_back(off);
		}

		auto schedules_buf = builder.CreateVector(schedules);

		auto trip_buf = buildTrip(builder, trip);

		nyc::realtime::NYCBusScheduleUpdateBuilder schedule_update_builder(builder);
		schedule_update_builder.add_trip(trip_buf);
		schedule_update_builder.add_schedule(schedules_buf);
		schedule_update_builder.add_timestamp((int)tu.timestamp);
		auto off = schedule_update_builder.Finish();

		builder.Finish(off);

		auto fbuffer = builder.GetBufferPointer();
		auto size = builder.GetSize();
		auto buffer = new unsigned char[size];

		/*std::string sbuf((char*)fbuffer, size);
		printf("\n%d BYTES BEFORE: ", size);
		std::cout << Andromeda::string_to_hex(sbuf) << std::endl;*/

		memcpy(buffer, fbuffer, size);

		message = buffer;
		message_len = size;
		return;
	}
	
	void NYCBusInterface::run() {
		running = true;
		while (running) {
			NYCBusTripEvent tracked_trip[32];
			auto count = holder->queue.wait_dequeue_bulk(tracked_trip, 32);

			if (wsInterface->has_json_clients > 0) {
				std::vector<std::pair<std::string /* command */, json11::Json>> data;

				for (int i = 0; i < count; i++) {
					auto trip = tracked_trip[i];

					json11::Json *j = nullptr;
					switch (trip.event_category) {
					case NYCBusTripEvent::StopChange:
						data.push_back(std::make_pair("StopChange", jBuildStopMessageUpdate(trip)));
						break;
					case NYCBusTripEvent::PositionChange:
						data.push_back(std::make_pair("PositionChange", jBuildStopMessageUpdate(trip)));
						break;
					case NYCBusTripEvent::ScheduleChange:
						//data.push_back(std::make_pair("ScheduleChange", jBuildTripScheduleUpdate(trip)));
						break;
					default:
						printf("Ignoring unknown event\n");
					}
				}

				if (data.size())
					this->wsInterface->broadcastJsonPreferred("bus", data);
			}

			if (wsInterface->has_flat_clients > 0) {
				std::vector<WSInterface::BinaryMessageWrapper> messages;
				for (int i = 0; i < count; i++) {
					auto trip = tracked_trip[i];

					switch (trip.event_category) {
					case NYCBusTripEvent::StopChange:
					{
						WSInterface::BinaryMessageWrapper w;
						w.message_type = WSInterface::NYCBus_StopChange;
						unsigned char *buffer = nullptr;
						int buffer_len = 0;
						fBuildStopMessageUpdate(trip, buffer, buffer_len);
						assert(buffer != nullptr);
						messages.push_back({ WSInterface::NYCBus_StopChange, buffer, buffer_len });
						break;
					}
					case NYCBusTripEvent::PositionChange:
					{
						WSInterface::BinaryMessageWrapper w;
						w.message_type = WSInterface::NYCBus_PositionChange;
						unsigned char *buffer = nullptr;
						int buffer_len = 0;
						fBuildStopMessageUpdate(trip, buffer, buffer_len);
						assert(buffer != nullptr);
						messages.push_back({ WSInterface::NYCBus_PositionChange, buffer, buffer_len });
						break;
					}
					case NYCBusTripEvent::ScheduleChange:
					{
						unsigned char *buffer = nullptr;
						int buffer_len = 0;
						fBuildTripScheduleUpdate(trip, buffer, buffer_len);
						assert(buffer != nullptr);
						printf("Buffer ptr: %d", buffer);
						messages.push_back({ WSInterface::NYCBus_ScheduleChange, buffer, buffer_len });
						break;
					}
					default:
						printf("Ignoring unknown event\n");
					}
				}

				if (messages.size())
					this->wsInterface->broadcastBinaryPreferredBatch(WSInterface::MessageType::SubscribtionFeed, messages);
				// Cleanup is already done for us in above function
			}
		}
	}

	void NYCBusInterface::pCurrentTripsRequest(WSInterface::Client client) {
		std::vector<WSInterface::BinaryMessageWrapper> messages;

		for (auto tracker : trackers) {
			for (auto tracked_trip : tracker->getTrackedTripsRef()) {
				NYCBusTripEvent fake_event;
				fake_event.event_category = NYCBusTripEvent::ScheduleChange;
				fake_event.initial_tracked_trip = &tracked_trip.second;
				fake_event.trip_update = tracked_trip.second.last_tracked_trip;
				unsigned char *message = nullptr;
				int message_len = 0;
				this->fBuildTripScheduleUpdate(fake_event, message, message_len);
				messages.push_back({ WSInterface::NYCBus_CurrentTripsResponse, message, message_len });
			}
		}

		this->wsInterface->broadcastBinaryPreferredBatch(WSInterface::MessageType::Response, messages);
	}
}