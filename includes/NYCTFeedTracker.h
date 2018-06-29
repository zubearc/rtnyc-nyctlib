#pragma once

#include "NYCTFeedService.h"

namespace nyctlib {
	class NYCTFeedTracker {
		NYCTFeedService currentFeed;
	public:
		NYCTFeedTracker() : currentFeed(NYCTFeedService()) {}

		void update();

		std::vector<NYCTTripUpdate> getTripsScheduledToArriveAtStop(std::string station_id);

		void printTripsScheduledToArriveAtStop(std::string station_id);
	};
}