#include "NYCTFeedTracker.h"

#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP Sleep
#else
#include <unistd.h>
#define SLEEP sleep
#endif

#define CH_GREEN "\33[0;91m"
#define CH_RED "\33[0;92m"
#define CRESET "\33[0m"

namespace nyctlib {
	bool NYCTFeedTracker::update() {
		feed->update();

		auto currentFeed = feed->getCurrentFeed();
		
		if (currentFeed == nullptr) {
			printf("Cannot update because currentFeed was nullptr.\n");
			return false;
		}

		auto unaccounted_trips = this->tracked_trips;
		std::map<std::string, NYCTTripUpdate> latest_tracked_trips;

		auto printTripTimeData = [](const NYCTTripTimeUpdate& l) {
			printf("stop_id=%s, arrival_time=%lld, depature_time=%lld, scheduled_track=%s, actual_track=%s",
				l.stop_id.c_str(), l.arrival_time, l.depature_time, l.scheduled_track.c_str(), l.actual_track.c_str());
		};

		auto compare = [&](const NYCTTripUpdate& l, const NYCTTripUpdate& r) -> bool {
			for (int i = 0; i < l.stop_time_updates.size(); i++) {
				auto lT = (NYCTTripTimeUpdate*)l.stop_time_updates[i].get();
				auto rT = (NYCTTripTimeUpdate*)r.stop_time_updates[i].get();
				if (!(lT->actual_track == rT->actual_track &&
					lT->arrival_time == rT->arrival_time &&
					lT->depature_time == rT->depature_time &&
					lT->scheduled_track == rT->scheduled_track &&
					lT->stop_id == rT->stop_id)) {
					printf("NYCTFeedTracker: TRIPCHANGE (for stop #%s)\n", i);
					printf("was: ");
					printTripTimeData(*lT);
					printf("\nnow: ");
					printTripTimeData(*rT);
					printf("\n");
					return false;
				}
			}
			return true;
		};

		// C++11 is fucking amazing -- NO TEMPLATES!!
		auto compareTripUpdates = [&](std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &old, std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &current) {
			
			std::map<std::string /* stop id */, NYCTTripTimeUpdate*> old_map;
			std::map<std::string /* stop id */, NYCTTripTimeUpdate*> current_map;

			_ASSERT(current.size());

			for (auto &i : old) {
				auto lT = (NYCTTripTimeUpdate*)i.get();
				old_map[lT->stop_id] = lT;
			}

			for (auto &i : current) {
				auto lT = (NYCTTripTimeUpdate*)i.get();
				current_map[lT->stop_id] = lT;
			}

			for (auto i : current_map) {
				auto lT = old_map[i.first];
				if (lT == nullptr) {
					printf("NYCTFeedTracker: NEWDATA for stop %s\n", i.first.c_str());
					printTripTimeData(*i.second);
					old_map.erase(i.first); // C++ creates elements with the [] operator, erase these.
											// We could do a .find() but since it is relatively rare for this block of code
											// to be called, it's probably more efficient to simply remove it here instead
											// of a double search of old_map

					continue;
				}
				auto rT = i.second;
				if (!(lT->actual_track == rT->actual_track &&
					lT->arrival_time == rT->arrival_time &&
					lT->depature_time == rT->depature_time &&
					lT->scheduled_track == rT->scheduled_track &&
					lT->stop_id == rT->stop_id)) {
					printf("NYCTFeedTracker: TRIPCHANGE (for stop #%s)", i.first.c_str());
					if ((lT->arrival_time != rT->arrival_time) && lT->arrival_time && rT->arrival_time) {
						long long arrival_diff = rT->arrival_time - lT->arrival_time;
						if (arrival_diff > 0) {
							printf(" - arrival %lld seconds delayed", arrival_diff);
						} else {
							printf(" - arrival %lld seconds early", arrival_diff);
						}
					}
					if ((lT->depature_time != rT->depature_time) && lT->depature_time && rT->depature_time) {
						long long depature_diff = rT->depature_time - lT->depature_time;
						if (depature_diff > 0) {
							printf(" - depature %lld seconds delayed", depature_diff);
						} else {
							printf(" - depature %lld seconds early", depature_diff);
						}
					}
					printf("\nwas: ");
					printTripTimeData(*lT);
					printf("\nnow: ");
					printTripTimeData(*rT);
					printf("\n");
				}
				old_map.erase(i.first);
			}

			for (auto i : old_map) {
				printf("NYCTFeedTracker: LOSTTRIPTIME - train probably passed this stop.\n");
				printTripTimeData(*i.second);
				printf("\n");
			}
		};

		currentFeed->forEachTripUpdate([&](NYCTTripUpdate *tu) {
			NYCTTrip *trip = (NYCTTrip*)tu->trip.get();
			std::string trainid = trip->nyct_train_id;
			
			if (/*this->tracked_ticker [don't remember what this is] */true) {
				if (this->tracked_trips.count(trainid) == 0) {
					printf("NYCTTrainTracker: " CH_RED "New untracked train with ID '%s'" CRESET "\n", trainid.c_str());
					this->tracked_trips[trainid] = NYCTTripUpdate(*tu);
				} else {
					//TODO
					//printf("NYCTTrainT")
					compareTripUpdates(tracked_trips[trainid].stop_time_updates, tu->stop_time_updates);
					unaccounted_trips.erase(trainid);
				}
			}
		});

		for (auto unaccounted_trip : unaccounted_trips) {
			printf("NYCTTrainTracker: " CH_GREEN "Lost track of trip '%s'" CRESET "\n", unaccounted_trip.first.c_str());
			this->tracked_trips.erase(unaccounted_trip.first);
			//unaccounted_trip.second.stop_time_updates.at(0)->stop_id;
		}
		return true;
	}

	void NYCTFeedTracker::run() {
		while (this->active) {
			bool ret = this->update();
			if (!ret)
				this->update(); // try once more if we fail first time
								// this may happen if the server goes breifly offline

			printf("done, going back to sleep. zzz.\n");
			SLEEP(15000);
		}
	}

	std::vector<NYCTTripUpdate> NYCTFeedTracker::getTripsScheduledToArriveAtStop(std::string stop_id) {
		std::vector<NYCTTripUpdate> trips;

		auto currentFeed = feed->getCurrentFeed();

		if (currentFeed == nullptr) {
			printf("Cannot update because currentFeed was nullptr.\n");
			return trips;
		}

		currentFeed->forEachTripUpdate([&](NYCTTripUpdate *tu) {
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
