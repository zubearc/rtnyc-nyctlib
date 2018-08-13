// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RTNYCPROTOCOL_NYC_REALTIME_H_
#define FLATBUFFERS_GENERATED_RTNYCPROTOCOL_NYC_REALTIME_H_

#include "flatbuffers/flatbuffers.h"

namespace nyc {
namespace realtime {

struct NYCSubwayTrip;

struct NYCSubwaySchedule;

struct NYCSubwayStopUpdate;

struct NYCSubwayScheduleUpdate;

struct NYCSubwayTrips;

enum TripStatus {
  TripStatus_AtStation = 0,
  TripStatus_ApproachingStation = 1,
  TripStatus_EnrouteToStation = 2,
  TripStatus_MIN = TripStatus_AtStation,
  TripStatus_MAX = TripStatus_EnrouteToStation
};

inline const TripStatus (&EnumValuesTripStatus())[3] {
  static const TripStatus values[] = {
    TripStatus_AtStation,
    TripStatus_ApproachingStation,
    TripStatus_EnrouteToStation
  };
  return values;
}

inline const char * const *EnumNamesTripStatus() {
  static const char * const names[] = {
    "AtStation",
    "ApproachingStation",
    "EnrouteToStation",
    nullptr
  };
  return names;
}

inline const char *EnumNameTripStatus(TripStatus e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesTripStatus()[index];
}

enum Direction {
  Direction_North = 0,
  Direction_South = 1,
  Direction_East = 2,
  Direction_West = 3,
  Direction_MIN = Direction_North,
  Direction_MAX = Direction_West
};

inline const Direction (&EnumValuesDirection())[4] {
  static const Direction values[] = {
    Direction_North,
    Direction_South,
    Direction_East,
    Direction_West
  };
  return values;
}

inline const char * const *EnumNamesDirection() {
  static const char * const names[] = {
    "North",
    "South",
    "East",
    "West",
    nullptr
  };
  return names;
}

inline const char *EnumNameDirection(Direction e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesDirection()[index];
}

struct NYCSubwayTrip FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_GTFS_TRIP_ID = 4,
    VT_TRIP_ID = 6,
    VT_START_TIME = 8,
    VT_ROUTE_ID = 10,
    VT_ASSIGNED = 12,
    VT_DIRECTION = 14
  };
  const flatbuffers::String *gtfs_trip_id() const {
    return GetPointer<const flatbuffers::String *>(VT_GTFS_TRIP_ID);
  }
  const flatbuffers::String *trip_id() const {
    return GetPointer<const flatbuffers::String *>(VT_TRIP_ID);
  }
  int32_t start_time() const {
    return GetField<int32_t>(VT_START_TIME, 0);
  }
  const flatbuffers::String *route_id() const {
    return GetPointer<const flatbuffers::String *>(VT_ROUTE_ID);
  }
  bool assigned() const {
    return GetField<uint8_t>(VT_ASSIGNED, 0) != 0;
  }
  Direction direction() const {
    return static_cast<Direction>(GetField<int8_t>(VT_DIRECTION, 0));
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_GTFS_TRIP_ID) &&
           verifier.Verify(gtfs_trip_id()) &&
           VerifyOffset(verifier, VT_TRIP_ID) &&
           verifier.Verify(trip_id()) &&
           VerifyField<int32_t>(verifier, VT_START_TIME) &&
           VerifyOffset(verifier, VT_ROUTE_ID) &&
           verifier.Verify(route_id()) &&
           VerifyField<uint8_t>(verifier, VT_ASSIGNED) &&
           VerifyField<int8_t>(verifier, VT_DIRECTION) &&
           verifier.EndTable();
  }
};

struct NYCSubwayTripBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_gtfs_trip_id(flatbuffers::Offset<flatbuffers::String> gtfs_trip_id) {
    fbb_.AddOffset(NYCSubwayTrip::VT_GTFS_TRIP_ID, gtfs_trip_id);
  }
  void add_trip_id(flatbuffers::Offset<flatbuffers::String> trip_id) {
    fbb_.AddOffset(NYCSubwayTrip::VT_TRIP_ID, trip_id);
  }
  void add_start_time(int32_t start_time) {
    fbb_.AddElement<int32_t>(NYCSubwayTrip::VT_START_TIME, start_time, 0);
  }
  void add_route_id(flatbuffers::Offset<flatbuffers::String> route_id) {
    fbb_.AddOffset(NYCSubwayTrip::VT_ROUTE_ID, route_id);
  }
  void add_assigned(bool assigned) {
    fbb_.AddElement<uint8_t>(NYCSubwayTrip::VT_ASSIGNED, static_cast<uint8_t>(assigned), 0);
  }
  void add_direction(Direction direction) {
    fbb_.AddElement<int8_t>(NYCSubwayTrip::VT_DIRECTION, static_cast<int8_t>(direction), 0);
  }
  explicit NYCSubwayTripBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  NYCSubwayTripBuilder &operator=(const NYCSubwayTripBuilder &);
  flatbuffers::Offset<NYCSubwayTrip> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<NYCSubwayTrip>(end);
    return o;
  }
};

inline flatbuffers::Offset<NYCSubwayTrip> CreateNYCSubwayTrip(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> gtfs_trip_id = 0,
    flatbuffers::Offset<flatbuffers::String> trip_id = 0,
    int32_t start_time = 0,
    flatbuffers::Offset<flatbuffers::String> route_id = 0,
    bool assigned = false,
    Direction direction = Direction_North) {
  NYCSubwayTripBuilder builder_(_fbb);
  builder_.add_route_id(route_id);
  builder_.add_start_time(start_time);
  builder_.add_trip_id(trip_id);
  builder_.add_gtfs_trip_id(gtfs_trip_id);
  builder_.add_direction(direction);
  builder_.add_assigned(assigned);
  return builder_.Finish();
}

inline flatbuffers::Offset<NYCSubwayTrip> CreateNYCSubwayTripDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *gtfs_trip_id = nullptr,
    const char *trip_id = nullptr,
    int32_t start_time = 0,
    const char *route_id = nullptr,
    bool assigned = false,
    Direction direction = Direction_North) {
  return nyc::realtime::CreateNYCSubwayTrip(
      _fbb,
      gtfs_trip_id ? _fbb.CreateString(gtfs_trip_id) : 0,
      trip_id ? _fbb.CreateString(trip_id) : 0,
      start_time,
      route_id ? _fbb.CreateString(route_id) : 0,
      assigned,
      direction);
}

struct NYCSubwaySchedule FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_SCHEDULED_TRACK = 4,
    VT_ACTUAL_TRACK = 6,
    VT_ARRIVAL_TIME = 8,
    VT_DEPARTURE_TIME = 10,
    VT_STOP_ID = 12
  };
  const flatbuffers::String *scheduled_track() const {
    return GetPointer<const flatbuffers::String *>(VT_SCHEDULED_TRACK);
  }
  const flatbuffers::String *actual_track() const {
    return GetPointer<const flatbuffers::String *>(VT_ACTUAL_TRACK);
  }
  int32_t arrival_time() const {
    return GetField<int32_t>(VT_ARRIVAL_TIME, 0);
  }
  int32_t departure_time() const {
    return GetField<int32_t>(VT_DEPARTURE_TIME, 0);
  }
  const flatbuffers::String *stop_id() const {
    return GetPointer<const flatbuffers::String *>(VT_STOP_ID);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SCHEDULED_TRACK) &&
           verifier.Verify(scheduled_track()) &&
           VerifyOffset(verifier, VT_ACTUAL_TRACK) &&
           verifier.Verify(actual_track()) &&
           VerifyField<int32_t>(verifier, VT_ARRIVAL_TIME) &&
           VerifyField<int32_t>(verifier, VT_DEPARTURE_TIME) &&
           VerifyOffset(verifier, VT_STOP_ID) &&
           verifier.Verify(stop_id()) &&
           verifier.EndTable();
  }
};

struct NYCSubwayScheduleBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_scheduled_track(flatbuffers::Offset<flatbuffers::String> scheduled_track) {
    fbb_.AddOffset(NYCSubwaySchedule::VT_SCHEDULED_TRACK, scheduled_track);
  }
  void add_actual_track(flatbuffers::Offset<flatbuffers::String> actual_track) {
    fbb_.AddOffset(NYCSubwaySchedule::VT_ACTUAL_TRACK, actual_track);
  }
  void add_arrival_time(int32_t arrival_time) {
    fbb_.AddElement<int32_t>(NYCSubwaySchedule::VT_ARRIVAL_TIME, arrival_time, 0);
  }
  void add_departure_time(int32_t departure_time) {
    fbb_.AddElement<int32_t>(NYCSubwaySchedule::VT_DEPARTURE_TIME, departure_time, 0);
  }
  void add_stop_id(flatbuffers::Offset<flatbuffers::String> stop_id) {
    fbb_.AddOffset(NYCSubwaySchedule::VT_STOP_ID, stop_id);
  }
  explicit NYCSubwayScheduleBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  NYCSubwayScheduleBuilder &operator=(const NYCSubwayScheduleBuilder &);
  flatbuffers::Offset<NYCSubwaySchedule> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<NYCSubwaySchedule>(end);
    return o;
  }
};

inline flatbuffers::Offset<NYCSubwaySchedule> CreateNYCSubwaySchedule(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> scheduled_track = 0,
    flatbuffers::Offset<flatbuffers::String> actual_track = 0,
    int32_t arrival_time = 0,
    int32_t departure_time = 0,
    flatbuffers::Offset<flatbuffers::String> stop_id = 0) {
  NYCSubwayScheduleBuilder builder_(_fbb);
  builder_.add_stop_id(stop_id);
  builder_.add_departure_time(departure_time);
  builder_.add_arrival_time(arrival_time);
  builder_.add_actual_track(actual_track);
  builder_.add_scheduled_track(scheduled_track);
  return builder_.Finish();
}

inline flatbuffers::Offset<NYCSubwaySchedule> CreateNYCSubwayScheduleDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *scheduled_track = nullptr,
    const char *actual_track = nullptr,
    int32_t arrival_time = 0,
    int32_t departure_time = 0,
    const char *stop_id = nullptr) {
  return nyc::realtime::CreateNYCSubwaySchedule(
      _fbb,
      scheduled_track ? _fbb.CreateString(scheduled_track) : 0,
      actual_track ? _fbb.CreateString(actual_track) : 0,
      arrival_time,
      departure_time,
      stop_id ? _fbb.CreateString(stop_id) : 0);
}

struct NYCSubwayStopUpdate FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TIMESTAMP = 4,
    VT_TRIP_ID = 6,
    VT_CURRENT_STATUS = 8,
    VT_CUMULATIVE_ARRIVAL_DELAY = 10,
    VT_CUMULATIVE_DEPARTURE_DELAY = 12,
    VT_ON_SCHEDULE = 14,
    VT_CURRENT_STOP_ID = 16,
    VT_CURRENT_STOP_DEPARTING_ON = 18,
    VT_NEXT_STOP_ID = 20,
    VT_NEXT_STOP_ARRIVING_ON = 22,
    VT_NEXT_STOP_DEPARTING_ON = 24,
    VT_LAST_STOP_ID = 26
  };
  int32_t timestamp() const {
    return GetField<int32_t>(VT_TIMESTAMP, 0);
  }
  const flatbuffers::String *trip_id() const {
    return GetPointer<const flatbuffers::String *>(VT_TRIP_ID);
  }
  TripStatus current_status() const {
    return static_cast<TripStatus>(GetField<int8_t>(VT_CURRENT_STATUS, 0));
  }
  int32_t cumulative_arrival_delay() const {
    return GetField<int32_t>(VT_CUMULATIVE_ARRIVAL_DELAY, 0);
  }
  int32_t cumulative_departure_delay() const {
    return GetField<int32_t>(VT_CUMULATIVE_DEPARTURE_DELAY, 0);
  }
  bool on_schedule() const {
    return GetField<uint8_t>(VT_ON_SCHEDULE, 0) != 0;
  }
  const flatbuffers::String *current_stop_id() const {
    return GetPointer<const flatbuffers::String *>(VT_CURRENT_STOP_ID);
  }
  int32_t current_stop_departing_on() const {
    return GetField<int32_t>(VT_CURRENT_STOP_DEPARTING_ON, 0);
  }
  const flatbuffers::String *next_stop_id() const {
    return GetPointer<const flatbuffers::String *>(VT_NEXT_STOP_ID);
  }
  int32_t next_stop_arriving_on() const {
    return GetField<int32_t>(VT_NEXT_STOP_ARRIVING_ON, 0);
  }
  int32_t next_stop_departing_on() const {
    return GetField<int32_t>(VT_NEXT_STOP_DEPARTING_ON, 0);
  }
  const flatbuffers::String *last_stop_id() const {
    return GetPointer<const flatbuffers::String *>(VT_LAST_STOP_ID);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_TIMESTAMP) &&
           VerifyOffset(verifier, VT_TRIP_ID) &&
           verifier.Verify(trip_id()) &&
           VerifyField<int8_t>(verifier, VT_CURRENT_STATUS) &&
           VerifyField<int32_t>(verifier, VT_CUMULATIVE_ARRIVAL_DELAY) &&
           VerifyField<int32_t>(verifier, VT_CUMULATIVE_DEPARTURE_DELAY) &&
           VerifyField<uint8_t>(verifier, VT_ON_SCHEDULE) &&
           VerifyOffset(verifier, VT_CURRENT_STOP_ID) &&
           verifier.Verify(current_stop_id()) &&
           VerifyField<int32_t>(verifier, VT_CURRENT_STOP_DEPARTING_ON) &&
           VerifyOffset(verifier, VT_NEXT_STOP_ID) &&
           verifier.Verify(next_stop_id()) &&
           VerifyField<int32_t>(verifier, VT_NEXT_STOP_ARRIVING_ON) &&
           VerifyField<int32_t>(verifier, VT_NEXT_STOP_DEPARTING_ON) &&
           VerifyOffset(verifier, VT_LAST_STOP_ID) &&
           verifier.Verify(last_stop_id()) &&
           verifier.EndTable();
  }
};

struct NYCSubwayStopUpdateBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_timestamp(int32_t timestamp) {
    fbb_.AddElement<int32_t>(NYCSubwayStopUpdate::VT_TIMESTAMP, timestamp, 0);
  }
  void add_trip_id(flatbuffers::Offset<flatbuffers::String> trip_id) {
    fbb_.AddOffset(NYCSubwayStopUpdate::VT_TRIP_ID, trip_id);
  }
  void add_current_status(TripStatus current_status) {
    fbb_.AddElement<int8_t>(NYCSubwayStopUpdate::VT_CURRENT_STATUS, static_cast<int8_t>(current_status), 0);
  }
  void add_cumulative_arrival_delay(int32_t cumulative_arrival_delay) {
    fbb_.AddElement<int32_t>(NYCSubwayStopUpdate::VT_CUMULATIVE_ARRIVAL_DELAY, cumulative_arrival_delay, 0);
  }
  void add_cumulative_departure_delay(int32_t cumulative_departure_delay) {
    fbb_.AddElement<int32_t>(NYCSubwayStopUpdate::VT_CUMULATIVE_DEPARTURE_DELAY, cumulative_departure_delay, 0);
  }
  void add_on_schedule(bool on_schedule) {
    fbb_.AddElement<uint8_t>(NYCSubwayStopUpdate::VT_ON_SCHEDULE, static_cast<uint8_t>(on_schedule), 0);
  }
  void add_current_stop_id(flatbuffers::Offset<flatbuffers::String> current_stop_id) {
    fbb_.AddOffset(NYCSubwayStopUpdate::VT_CURRENT_STOP_ID, current_stop_id);
  }
  void add_current_stop_departing_on(int32_t current_stop_departing_on) {
    fbb_.AddElement<int32_t>(NYCSubwayStopUpdate::VT_CURRENT_STOP_DEPARTING_ON, current_stop_departing_on, 0);
  }
  void add_next_stop_id(flatbuffers::Offset<flatbuffers::String> next_stop_id) {
    fbb_.AddOffset(NYCSubwayStopUpdate::VT_NEXT_STOP_ID, next_stop_id);
  }
  void add_next_stop_arriving_on(int32_t next_stop_arriving_on) {
    fbb_.AddElement<int32_t>(NYCSubwayStopUpdate::VT_NEXT_STOP_ARRIVING_ON, next_stop_arriving_on, 0);
  }
  void add_next_stop_departing_on(int32_t next_stop_departing_on) {
    fbb_.AddElement<int32_t>(NYCSubwayStopUpdate::VT_NEXT_STOP_DEPARTING_ON, next_stop_departing_on, 0);
  }
  void add_last_stop_id(flatbuffers::Offset<flatbuffers::String> last_stop_id) {
    fbb_.AddOffset(NYCSubwayStopUpdate::VT_LAST_STOP_ID, last_stop_id);
  }
  explicit NYCSubwayStopUpdateBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  NYCSubwayStopUpdateBuilder &operator=(const NYCSubwayStopUpdateBuilder &);
  flatbuffers::Offset<NYCSubwayStopUpdate> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<NYCSubwayStopUpdate>(end);
    return o;
  }
};

inline flatbuffers::Offset<NYCSubwayStopUpdate> CreateNYCSubwayStopUpdate(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t timestamp = 0,
    flatbuffers::Offset<flatbuffers::String> trip_id = 0,
    TripStatus current_status = TripStatus_AtStation,
    int32_t cumulative_arrival_delay = 0,
    int32_t cumulative_departure_delay = 0,
    bool on_schedule = false,
    flatbuffers::Offset<flatbuffers::String> current_stop_id = 0,
    int32_t current_stop_departing_on = 0,
    flatbuffers::Offset<flatbuffers::String> next_stop_id = 0,
    int32_t next_stop_arriving_on = 0,
    int32_t next_stop_departing_on = 0,
    flatbuffers::Offset<flatbuffers::String> last_stop_id = 0) {
  NYCSubwayStopUpdateBuilder builder_(_fbb);
  builder_.add_last_stop_id(last_stop_id);
  builder_.add_next_stop_departing_on(next_stop_departing_on);
  builder_.add_next_stop_arriving_on(next_stop_arriving_on);
  builder_.add_next_stop_id(next_stop_id);
  builder_.add_current_stop_departing_on(current_stop_departing_on);
  builder_.add_current_stop_id(current_stop_id);
  builder_.add_cumulative_departure_delay(cumulative_departure_delay);
  builder_.add_cumulative_arrival_delay(cumulative_arrival_delay);
  builder_.add_trip_id(trip_id);
  builder_.add_timestamp(timestamp);
  builder_.add_on_schedule(on_schedule);
  builder_.add_current_status(current_status);
  return builder_.Finish();
}

inline flatbuffers::Offset<NYCSubwayStopUpdate> CreateNYCSubwayStopUpdateDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t timestamp = 0,
    const char *trip_id = nullptr,
    TripStatus current_status = TripStatus_AtStation,
    int32_t cumulative_arrival_delay = 0,
    int32_t cumulative_departure_delay = 0,
    bool on_schedule = false,
    const char *current_stop_id = nullptr,
    int32_t current_stop_departing_on = 0,
    const char *next_stop_id = nullptr,
    int32_t next_stop_arriving_on = 0,
    int32_t next_stop_departing_on = 0,
    const char *last_stop_id = nullptr) {
  return nyc::realtime::CreateNYCSubwayStopUpdate(
      _fbb,
      timestamp,
      trip_id ? _fbb.CreateString(trip_id) : 0,
      current_status,
      cumulative_arrival_delay,
      cumulative_departure_delay,
      on_schedule,
      current_stop_id ? _fbb.CreateString(current_stop_id) : 0,
      current_stop_departing_on,
      next_stop_id ? _fbb.CreateString(next_stop_id) : 0,
      next_stop_arriving_on,
      next_stop_departing_on,
      last_stop_id ? _fbb.CreateString(last_stop_id) : 0);
}

struct NYCSubwayScheduleUpdate FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TIMESTAMP = 4,
    VT_TRIP = 6,
    VT_SCHEDULE = 8
  };
  int32_t timestamp() const {
    return GetField<int32_t>(VT_TIMESTAMP, 0);
  }
  const NYCSubwayTrip *trip() const {
    return GetPointer<const NYCSubwayTrip *>(VT_TRIP);
  }
  const flatbuffers::Vector<flatbuffers::Offset<NYCSubwaySchedule>> *schedule() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<NYCSubwaySchedule>> *>(VT_SCHEDULE);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_TIMESTAMP) &&
           VerifyOffset(verifier, VT_TRIP) &&
           verifier.VerifyTable(trip()) &&
           VerifyOffset(verifier, VT_SCHEDULE) &&
           verifier.Verify(schedule()) &&
           verifier.VerifyVectorOfTables(schedule()) &&
           verifier.EndTable();
  }
};

struct NYCSubwayScheduleUpdateBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_timestamp(int32_t timestamp) {
    fbb_.AddElement<int32_t>(NYCSubwayScheduleUpdate::VT_TIMESTAMP, timestamp, 0);
  }
  void add_trip(flatbuffers::Offset<NYCSubwayTrip> trip) {
    fbb_.AddOffset(NYCSubwayScheduleUpdate::VT_TRIP, trip);
  }
  void add_schedule(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<NYCSubwaySchedule>>> schedule) {
    fbb_.AddOffset(NYCSubwayScheduleUpdate::VT_SCHEDULE, schedule);
  }
  explicit NYCSubwayScheduleUpdateBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  NYCSubwayScheduleUpdateBuilder &operator=(const NYCSubwayScheduleUpdateBuilder &);
  flatbuffers::Offset<NYCSubwayScheduleUpdate> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<NYCSubwayScheduleUpdate>(end);
    return o;
  }
};

inline flatbuffers::Offset<NYCSubwayScheduleUpdate> CreateNYCSubwayScheduleUpdate(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t timestamp = 0,
    flatbuffers::Offset<NYCSubwayTrip> trip = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<NYCSubwaySchedule>>> schedule = 0) {
  NYCSubwayScheduleUpdateBuilder builder_(_fbb);
  builder_.add_schedule(schedule);
  builder_.add_trip(trip);
  builder_.add_timestamp(timestamp);
  return builder_.Finish();
}

inline flatbuffers::Offset<NYCSubwayScheduleUpdate> CreateNYCSubwayScheduleUpdateDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t timestamp = 0,
    flatbuffers::Offset<NYCSubwayTrip> trip = 0,
    const std::vector<flatbuffers::Offset<NYCSubwaySchedule>> *schedule = nullptr) {
  return nyc::realtime::CreateNYCSubwayScheduleUpdate(
      _fbb,
      timestamp,
      trip,
      schedule ? _fbb.CreateVector<flatbuffers::Offset<NYCSubwaySchedule>>(*schedule) : 0);
}

struct NYCSubwayTrips FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TIMESTAMP = 4,
    VT_TRIPS = 6
  };
  int32_t timestamp() const {
    return GetField<int32_t>(VT_TIMESTAMP, 0);
  }
  const flatbuffers::Vector<flatbuffers::Offset<NYCSubwayScheduleUpdate>> *trips() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<NYCSubwayScheduleUpdate>> *>(VT_TRIPS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_TIMESTAMP) &&
           VerifyOffset(verifier, VT_TRIPS) &&
           verifier.Verify(trips()) &&
           verifier.VerifyVectorOfTables(trips()) &&
           verifier.EndTable();
  }
};

struct NYCSubwayTripsBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_timestamp(int32_t timestamp) {
    fbb_.AddElement<int32_t>(NYCSubwayTrips::VT_TIMESTAMP, timestamp, 0);
  }
  void add_trips(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<NYCSubwayScheduleUpdate>>> trips) {
    fbb_.AddOffset(NYCSubwayTrips::VT_TRIPS, trips);
  }
  explicit NYCSubwayTripsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  NYCSubwayTripsBuilder &operator=(const NYCSubwayTripsBuilder &);
  flatbuffers::Offset<NYCSubwayTrips> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<NYCSubwayTrips>(end);
    return o;
  }
};

inline flatbuffers::Offset<NYCSubwayTrips> CreateNYCSubwayTrips(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t timestamp = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<NYCSubwayScheduleUpdate>>> trips = 0) {
  NYCSubwayTripsBuilder builder_(_fbb);
  builder_.add_trips(trips);
  builder_.add_timestamp(timestamp);
  return builder_.Finish();
}

inline flatbuffers::Offset<NYCSubwayTrips> CreateNYCSubwayTripsDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    int32_t timestamp = 0,
    const std::vector<flatbuffers::Offset<NYCSubwayScheduleUpdate>> *trips = nullptr) {
  return nyc::realtime::CreateNYCSubwayTrips(
      _fbb,
      timestamp,
      trips ? _fbb.CreateVector<flatbuffers::Offset<NYCSubwayScheduleUpdate>>(*trips) : 0);
}

}  // namespace realtime
}  // namespace nyc

#endif  // FLATBUFFERS_GENERATED_RTNYCPROTOCOL_NYC_REALTIME_H_
