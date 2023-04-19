#ifdef SESS
#include "interfaces/SessInterface.h"

#include "sess_generated.h"

using namespace nyc::realtime;

namespace nyctlib {

	std::string sanitize_word(std::string inp) {
		std::string replacement;
		for (auto c : inp) {
			if (isalnum(c)) {
				replacement += c;
			} else {
				replacement += '_';
			}
		}
		return replacement;
	}

	void SessInterface::commit(std::string nyct_trip_id, int timestamp, bool isDirectionSouth, 
		std::map<std::string, std::vector<SubwayTrackedTrip::ConfirmedStop>> &subway_trip_paths) {
		flatbuffers::FlatBufferBuilder builder(1024);

		//auto _trip_id = builder.CreateString(e.trip_id);
		flatbuffers::Offset<flatbuffers::String> _current_stop_id;
		flatbuffers::Offset<flatbuffers::String> _next_stop_id;
		flatbuffers::Offset<flatbuffers::String> _last_stop_id;

		std::vector<flatbuffers::Offset<SubwayTripPath>> trip_paths;

		for (auto subway_trip_path : subway_trip_paths) {
			auto _trip_path = builder.CreateString(subway_trip_path.first);
			std::vector<flatbuffers::Offset<SubwayTripStop>> confirmed_stops;

			for (auto confirmed_stop : subway_trip_path.second) {
				auto _stop_id = builder.CreateString(confirmed_stop.stop_id);
				auto _arrival_track = builder.CreateString(confirmed_stop.arrival_track);
				SubwayTripStopBuilder trip_stop_buffer(builder);
				trip_stop_buffer.add_stop_id(_stop_id);
				trip_stop_buffer.add_arrival_track(_arrival_track);
				trip_stop_buffer.add_stop_sequence(confirmed_stop.stop_sequence);
				trip_stop_buffer.add_confidence(1);
				trip_stop_buffer.add_on(timestamp - confirmed_stop.timestamp);
				auto tsb = trip_stop_buffer.Finish();
				confirmed_stops.push_back(tsb);
			}

			auto confirmed_stops_buf = builder.CreateVector(confirmed_stops);

			SubwayTripPathBuilder subway_trip_path_buffer(builder);
			subway_trip_path_buffer.add_stops(confirmed_stops_buf);
			subway_trip_path_buffer.add_trip_id(_trip_path);
			auto buf = subway_trip_path_buffer.Finish();
			trip_paths.push_back(buf);
		}

		auto trip_paths_buf = builder.CreateVector(trip_paths);

		auto _nyct_trip_id = builder.CreateString(nyct_trip_id);
		
		SubwayTripBuilder subway_trip_builder(builder);
		subway_trip_builder.add_direction(isDirectionSouth);
		subway_trip_builder.add_nyct_trip_id(_nyct_trip_id);
		subway_trip_builder.add_paths(trip_paths_buf);
		subway_trip_builder.add_timestamp(timestamp);

		auto off = subway_trip_builder.Finish();
		builder.Finish(off);
		auto fbuffer = builder.GetBufferPointer();
		int size = builder.GetSize();

		std::string file_name = "SESS_" + sanitize_word(nyct_trip_id) + "_" +  std::to_string(timestamp) + ".fb";

		auto fp = fopen(file_name.c_str(), "wb");
		if (!fp) {
			fprintf(stderr, "SessInterface: Failed to write SESS file to ./[%s]\n", file_name.c_str());
			return;
		}
		fwrite(fbuffer, 1, size, fp);
		fclose(fp);
	}

	void SessInterface::concatenate()
	{
	}
}
#endif