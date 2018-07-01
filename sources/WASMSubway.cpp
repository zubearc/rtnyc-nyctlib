#include "../includes/WASMSubway.h"

#include <stdio.h>

#include "NYCTFeedTracker.h"

using namespace nyctlib;

EXPORTABLE void nyctlib_init() {
	printf("Hello from WASMSubway.cpp!\n");
}

EXPORTABLE PNYCTFeedTracker nyctlib_NYCTFeedService_create() {
	auto tracker = new NYCTFeedTracker();
	return tracker;
}

EXPORTABLE bool nyctlib_NYCTFeedService_loadbuffer(PNYCTFeedTracker tracker, const char *buffer) {
	auto trip_update = (NYCTTripUpdate*)tracker._NYCTTripUpdate;
}

EXPORTABLE void nyctlib_NYCTFeedService_destroy(PNYCTFeedTracker tracker) {
	delete tracker->_NYCTTripUpdate;
}
