#include <future>
#include "interfaces/NYCTSubwayInterface.h"

#include "rtnyc_protocol_generated.h"

using namespace json11;

namespace nyctlib {
	NYCTSubwayInterface::NYCTSubwayInterface(WSInterface *ws_interface, std::vector<NYCTFeedTracker*> trackers,
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
		this->trackers = trackers;
	}

	void NYCTSubwayInterface::processRequest(WSInterface::Client client, std::string request, std::string paramaters) {
		if (request == "get-trip-update") {
			pLatestTripUpdateRequest(client, paramaters);
		} else if (request == "get-current-trips") {
			pCurrentTripsRequest(client);
		} else {
			this->wsInterface->respondError(client, "no such command");
		}
	}

	void NYCTSubwayInterface::processFlatRequest(WSInterface::Client client, char *message, int message_len) {

		//nyc::realtime::NYCSubwayStopUpdate::Verify()

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

	json11::Json jBuildTripCompleteUpdate(SubwayTripEvent &e) {
		auto tid = e.trip_id;

		Json::object json;
		json["tripid"] = tid;
		json11::Json j(json);
		return j;
	}

	void NYCTSubwayInterface::fBuildStopMessageUpdate(SubwayTripEvent &e, unsigned char* &message, int &message_len) {
		
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

		flatbuffers::FlatBufferBuilder builder(1024);

		// We must allocate everything before building the final buffer
		auto _trip_id = builder.CreateString(e.trip_id);
		flatbuffers::Offset<flatbuffers::String> _current_stop_id;
		flatbuffers::Offset<flatbuffers::String> _next_stop_id;
		flatbuffers::Offset<flatbuffers::String> _last_stop_id;

		bool _has_next_stop = false;
		bool _has_last_stop = false;
		int _next_stop_arrival_time;
		int _next_stop_departure_time;

		if (vu->stop_progress == GtfsVehicleProgress::AtStation) {
			_current_stop_id =  builder.CreateString(vu->stop_id);

			auto next_stop = e.initial_tracked_trip->getNextStopScheduled(vu->stop_id);

			if (next_stop) {
				_has_next_stop = true;
				_next_stop_id = builder.CreateString(next_stop->stop_id);
				_next_stop_arrival_time = (int)next_stop->arrival_time;
				_next_stop_departure_time = (int)next_stop->depature_time;
			}
		} else {
			NYCTTripTimeUpdate *last_stop = e.initial_tracked_trip->getPreviousStopScheduled(vu->stop_id);

			if (last_stop != nullptr) {
				if (last_stop->stop_id.size() > 0) {
					_has_last_stop = true;
					_last_stop_id = builder.CreateString(last_stop->stop_id);
				}
			}

			_next_stop_id = builder.CreateString(vu->stop_id);
		}

		nyc::realtime::NYCSubwayStopUpdateBuilder stop_update(builder);
		auto ts = tracked->last_update_time ? (int)tracked->last_update_time : tracked->last_tracked_trip.timestamp;
		stop_update.add_timestamp(ts);
		stop_update.add_trip_id(_trip_id);

		nyc::realtime::TripStatus tripstatus = *(nyc::realtime::TripStatus*)&vu->stop_progress;
		stop_update.add_current_status(tripstatus);
		stop_update.add_cumulative_arrival_delay(tracked->cumulative_arrival_delay);
		stop_update.add_cumulative_departure_delay(tracked->cumulative_depature_delay);
		stop_update.add_on_schedule(tracked->updated_trip_schedules.size() == 0 ? true : false);

		if (vu->stop_progress == GtfsVehicleProgress::AtStation) {
			stop_update.add_current_stop_id(_current_stop_id);
			stop_update.add_current_stop_departing_on(current_depature_time);

			if (_has_next_stop) {
				stop_update.add_next_stop_id(_next_stop_id);
				stop_update.add_next_stop_arriving_on(_next_stop_arrival_time);
				stop_update.add_next_stop_departing_on(_next_stop_departure_time);
			}
		} else {

			if (_has_last_stop) {
				stop_update.add_last_stop_id(_last_stop_id);
			}

			stop_update.add_next_stop_id(_next_stop_id);
			stop_update.add_next_stop_arriving_on(current_arrival_time);
			stop_update.add_next_stop_departing_on(current_depature_time);
		}

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

	flatbuffers::Offset<nyc::realtime::NYCSubwayTrip> buildTrip(flatbuffers::FlatBufferBuilder &builder, NYCTTrip *trip) {
		auto _trip_id = builder.CreateString(trip->trip_id);
		auto _nyct_train_id = builder.CreateString(trip->nyct_train_id);
		auto _trip_route_id = builder.CreateString(trip->route_id);

		nyc::realtime::NYCSubwayTripBuilder trip_buffer(builder);
		trip_buffer.add_gtfs_trip_id(_trip_id);
		trip_buffer.add_trip_id(_nyct_train_id);
		trip_buffer.add_assigned(trip->nyct_is_assigned);
		trip_buffer.add_route_id(_trip_route_id);
		//trip_buffer.add_direction(0); // unused for now
		return trip_buffer.Finish();
	}

	flatbuffers::Offset<nyc::realtime::NYCSubwayVehiclePosition> buildVehiclePosition(flatbuffers::FlatBufferBuilder &builder, NYCTVehicleUpdate &vu, std::string tkid = "") {
		auto _stop_id = builder.CreateString(vu.stop_id);
		auto _track_id = builder.CreateString(tkid);
		
		nyc::realtime::NYCSubwayVehiclePositionBuilder vpos_buffer(builder);
		nyc::realtime::TripStatus ts = *(nyc::realtime::TripStatus*)&vu.stop_progress;
		vpos_buffer.add_relative_status(ts);
		vpos_buffer.add_stop_id(_stop_id);
		vpos_buffer.add_stop_index(vu.current_stop_index);
		vpos_buffer.add_track(_track_id);
		return vpos_buffer.Finish();
	}

	void NYCTSubwayInterface::fBuildTripScheduleUpdate(SubwayTripEvent &e, unsigned char* &message, int &message_len) {
		auto tu = e.trip_update;
		NYCTTrip *trip = (NYCTTrip*)tu.trip.get();
		auto vehicle = e.initial_tracked_trip->last_tracked_vehicle;
		
		flatbuffers::FlatBufferBuilder builder(1024);
		
		std::vector<flatbuffers::Offset<nyc::realtime::NYCSubwaySchedule>> schedules;
		auto last_stop_id = vehicle.stop_id;
		std::string on_track;
		
		for (auto &stu : tu.stop_time_updates) {
			auto s = (NYCTTripTimeUpdate*)stu.get();

			auto _scheduled_track = builder.CreateString(s->scheduled_track);
			auto _actual_track = builder.CreateString(s->actual_track);
			auto _stop_id = builder.CreateString(s->stop_id);

			if (last_stop_id.length() && s->stop_id == last_stop_id) {
				if (s->actual_track.length()) {
					on_track = s->actual_track;
				} else {
					on_track = s->scheduled_track;
				}
			}

			nyc::realtime::NYCSubwayScheduleBuilder schedule_buffer(builder);
			schedule_buffer.add_scheduled_track(_scheduled_track);
			schedule_buffer.add_actual_track(_actual_track);
			schedule_buffer.add_arrival_time((int)s->arrival_time);
			schedule_buffer.add_departure_time((int)s->depature_time);
			schedule_buffer.add_stop_id(_stop_id);
			auto off = schedule_buffer.Finish();
			schedules.push_back(off);
		}
		auto schedules_buf = builder.CreateVector(schedules);

		auto trip_buf = buildTrip(builder, trip);

		auto vehicle_position = buildVehiclePosition(builder, vehicle, on_track);

		nyc::realtime::NYCSubwayScheduleUpdateBuilder schedule_update_builder(builder);
		schedule_update_builder.add_trip(trip_buf);
		schedule_update_builder.add_schedule(schedules_buf);
			schedule_update_builder.add_timestamp((int)tu.timestamp);
		schedule_update_builder.add_last_vehicle_position(vehicle_position);
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

	void fBuildTripCompleteUpdate(SubwayTripEvent &e, unsigned char* &message, int &message_len) {
		flatbuffers::FlatBufferBuilder builder(1024);

		auto _trip_id = builder.CreateString(e.trip_id);

		nyc::realtime::NYCSubwayTripCompleteBuilder trip_complete_builder(builder);
		trip_complete_builder.add_trip_id(_trip_id);

		auto off = trip_complete_builder.Finish();

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

	void NYCTSubwayInterface::run() {
		running = true;
		while (running) {
			SubwayTripEvent tracked_trip[32];
			auto count = holder->queue.wait_dequeue_bulk(tracked_trip, 32);

			if (wsInterface->has_json_clients > 0)
			{
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
					case SubwayTripEvent::TripComplete:
						data.push_back(std::make_pair("TripComplete", jBuildTripCompleteUpdate(trip)));
						break;
					default:
						printf("Ignoring unknown event\n");
					}
				}
				this->wsInterface->broadcastJsonPreferred("subway", data);
			}

			if (wsInterface->has_flat_clients > 0) {
				std::vector<WSInterface::BinaryMessageWrapper> messages;
				for (int i = 0; i < count; i++) {
					auto trip = tracked_trip[i];

					switch (trip.event_category) {
					case SubwayTripEvent::StopChange:
					{
						WSInterface::BinaryMessageWrapper w;
						w.message_type = WSInterface::NYCSubway_StopChange;
						unsigned char *buffer = nullptr;
						int buffer_len = 0;
						fBuildStopMessageUpdate(trip, buffer, buffer_len);
						assert(buffer != nullptr);
						messages.push_back({ WSInterface::NYCSubway_StopChange, buffer, buffer_len });
						/*this->wsInterface->broadcastBinaryPreferred(
							WSInterface::NYCSubway_StopChange, (char*)buffer, buffer_len);
						delete[] buffer;*/
						break;
					}
					case SubwayTripEvent::TripAssigned:
					{
						unsigned char *buffer = nullptr;
						int buffer_len = 0;
						fBuildTripScheduleUpdate(trip, buffer, buffer_len);
						assert(buffer != nullptr);
						messages.push_back({ WSInterface::NYCSubway_TripAssigned, buffer, buffer_len });
						/*this->wsInterface->broadcastBinaryPreferred(
							WSInterface::NYCSubway_TripAssigned, (char*)buffer, buffer_len);
						delete[] buffer;*/
						break;
					}
					case SubwayTripEvent::ScheduleChange:
					{
						unsigned char *buffer = nullptr;
						int buffer_len = 0;
						fBuildTripScheduleUpdate(trip, buffer, buffer_len);
						messages.push_back({ WSInterface::NYCSubway_ScheduleChange, buffer, buffer_len });
						break;
					}
					case SubwayTripEvent::TripComplete:
					{
						unsigned char *buffer = nullptr;
						int buffer_len = 0;
						fBuildTripCompleteUpdate(trip, buffer, buffer_len);
						assert(buffer != nullptr);
						printf("Buffer ptr: %d", buffer);
						messages.push_back({ WSInterface::NYCSubway_TripComplete, buffer, buffer_len });
						break;
					}
					default:
						printf("Ignoring unknown event\n");
					}
				}

				this->wsInterface->broadcastBinaryPreferredBatch(WSInterface::MessageType::SubscribtionFeed, messages);
				// Cleanup is already done for us in above function
				/*for (auto message : messages) {
					delete[] *message.message;
				}*/
			}
		}
	}

	void NYCTSubwayInterface::pLatestTripUpdateRequest(WSInterface::Client client, std::string trip_id) {
		try {
			auto tu = trackers[0]->getTrackedTripUpdate(trip_id);
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

	void NYCTSubwayInterface::pCurrentTripsRequest(WSInterface::Client client) {
		std::vector<WSInterface::BinaryMessageWrapper> messages;

		for (auto tracker : trackers) {
			for (auto tracked_trip : tracker->getTrackedTripsRef()) {
				SubwayTripEvent fake_event;
				fake_event.event_category = SubwayTripEvent::TripAssigned;
				fake_event.initial_tracked_trip = &tracked_trip.second;
				fake_event.trip_update = tracked_trip.second.last_tracked_trip;
				unsigned char *message = nullptr;
				int message_len = 0;
				this->fBuildTripScheduleUpdate(fake_event, message, message_len);
				messages.push_back({ WSInterface::NYCSubway_CurrentTripsResponse, message, message_len });
			}
		}
		
		this->wsInterface->broadcastBinaryPreferredBatch(WSInterface::MessageType::Response, messages);
		// TODO: Make it so we don't broadcast to everyone
	}
}