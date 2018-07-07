#pragma once

#include <emscripten.h>
#include <emscripten/bind.h>

#define EXPORTABLE EMSCRIPTEN_KEEPALIVE
#ifndef STRING
#define STRING std::string
#define LIST(x) std::vector<x>
#endif

// Structs are pretty complicated to get working with Emscripten currently!
// We don't use them currently in favor of JS callbacks with paramaters
#if 0
    struct WNYCTTripTimeUpdate {
        long long   arrival_time;
        long long   depature_time;
        STRING      stop_id;
        STRING      scheduled_track;
        STRING      actual_track;
    };

    struct WNYCTSubwayTripUpdate {
        long long	timestamp;

        STRING      trip_id;
        STRING      trip_start_date;
        STRING      trip_route_id;
        STRING      trip_nyct_train_id;
        bool        trip_nyct_is_assigned;
        STRING      trip_nyct_direction;

        int         updates_count;
        LIST(WNYCTTripTimeUpdate) nyct_trip_time_update;
    };

    WNYCTSubwayTripUpdate GetWNYCTSubwayTripUpdate(void *ptr) {
        return WNYCTSubwayTripUpdate(*((WNYCTSubwayTripUpdate*)ptr));
    }

    EMSCRIPTEN_BINDINGS(sNycTripUpdate) {
        emscripten::value_object<WNYCTSubwayTripUpdate>("NYCTSubwayTripUpdate")
            .field("timestamp", &WNYCTSubwayTripUpdate::timestamp)
            .field("trip_id", &WNYCTSubwayTripUpdate::trip_id)
            .field("trip_start_date",&WNYCTSubwayTripUpdate::trip_start_date)
            .field("trip_route_id",&WNYCTSubwayTripUpdate::trip_route_id)
            .field("trip_nyct_train_id",&WNYCTSubwayTripUpdate::trip_nyct_train_id)
            .field("trip_nyct_is_assigned",&WNYCTSubwayTripUpdate::trip_nyct_is_assigned)
            .field("trip_nyct_direction",&WNYCTSubwayTripUpdate::trip_nyct_direction)
            .field("updates_count", &WNYCTSubwayTripUpdate::updates_count);
    }

    EMSCRIPTEN_BINDINGS(sNyctTripTimeUpdate) {
        emscripten::value_object<WNYCTTripTimeUpdate>("NYCTTripTimeUpdate")
            .field("arrival_time", &WNYCTTripTimeUpdate::arrival_time)
            .field("depature_time", &WNYCTTripTimeUpdate::depature_time)
            .field("stop_id", &WNYCTTripTimeUpdate::stop_id)
            .field("scheduled_track", &WNYCTTripTimeUpdate::scheduled_track)
            .field("actual_track", &WNYCTTripTimeUpdate::actual_track);
    }
    
#endif


#ifdef __cplusplus
extern "C" {
#endif

    // Struct that will be returned to the user
    // note: need to change std::string to char*
    // Container to pointers
    struct CNYCTSubwayTripUpdate {
        void *_NYCTTripUpdate;
    };

    // Return value pointer type mappings to classes
    typedef void* PNYCTFeedTracker;

    EXPORTABLE void nyctlib_init();

    EXPORTABLE PNYCTFeedTracker nyctlib_NYCTFeedTracker_create();

    EXPORTABLE bool nyctlib_NYCTFeedTracker_loadbuffer(PNYCTFeedTracker tracker, const char *buffer);

    EXPORTABLE bool nyctlib_NYCTFeedTracker_updateFromWeb(PNYCTFeedTracker tracker);

    EXPORTABLE bool nyctlib_NYCTFeedTracker_printTripsScheduledToArriveAtStop(PNYCTFeedTracker tracker, const char *station_id);

    typedef void (*tripCallback)(long long timestamp, const char *trip_id, const char *trip_start_time, const char *route_id, const char *trip_nyct_train_id, bool trip_nyct_is_assigned, const char *trip_nyct_direction);
    typedef void (*tripTimeUpdateCallback)(long long arrival_time, long long depature_time, const char *stop_id, char scheduled_track, char expected_track);

    EXPORTABLE bool nyctlib_NYCTFeedTracker_forEachTripScheduledToStopAt(PNYCTFeedTracker tracker, const char *station_id, tripCallback callback);

    EXPORTABLE bool nyctlib_NYCTFeedTracker_forEachTimeUpdateForTrip(PNYCTFeedTracker tracker, const char *nyct_trip_id, tripTimeUpdateCallback callback);

    EXPORTABLE void nyctlib_NYCTFeedTracker_destroy(PNYCTFeedTracker tracker);

#ifdef __cplusplus
}
#endif