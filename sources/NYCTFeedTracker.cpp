#include "NYCTFeedTracker.h"

#include <time.h>

namespace nyctlib {
	std::vector<NYCTTripUpdate> NYCTFeedTracker::getTripsScheduledToArriveAtStop(std::string stop_id) {
		std::vector<NYCTTripUpdate> trips;

		currentFeed.getCurrentFeed()->forEachTripUpdate([&](NYCTTripUpdate *tu) {
			for (auto timeUpdate : tu->stop_time_updates) {
				if (timeUpdate->stop_id == stop_id) {
					trips.push_back(NYCTTripUpdate(*tu));
				}
			}
		});

		return trips;
	}
	void NYCTFeedTracker::printTripsScheduledToArriveAtStop(std::string stop_id) {
		auto tripupdates = this->getTripsScheduledToArriveAtStop(stop_id);
		for (auto tripupdate : tripupdates) {
			auto nycttrip = (NYCTTrip*)tripupdate.trip.get();
			for (auto stoptimeupdate : tripupdate.stop_time_updates) {
				auto nyctStopTime = (NYCTTripTimeUpdate*)stoptimeupdate.get();
				if (nyctStopTime->stop_id != stop_id)
					continue;
				auto arrivaltime = nyctStopTime->arrival_time;
				if (arrivaltime > 0) {
					nycttrip->trip_id;
					long long currtime = time(NULL);
					long long secremaining = arrivaltime - currtime;
					printf("Trip '%s' will arrive at stop %s in %lld minutes\n", nycttrip->nyct_train_id.c_str(), stop_id.c_str(), secremaining / 60);
				}
			}
		}
	}
}