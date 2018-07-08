#include "NYCTFeedTracker.h"

#include <time.h>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#define SLEEP Sleep
#else
#include <unistd.h>
#define SLEEP(x) usleep(x * 1000)
#endif

#define CH_GREEN "\33[0;91m"
#define CH_RED "\33[0;92m"
#define CH_YELLOW "\33[0;93m"
#define CH_MAGENTA "\33[95m"
#define CPURPLE "\33[0;35m"
#define CGREEN "\33[0;32m"
#define CYELLOW "\33[0;33m"
#define CMAGENTA "\33[35m"
#define CWHITE "\33[1;37m"
#define CRESET "\33[0m"

namespace nyctlib {
	void NYCTFeedTracker::clearTrackedDataForTrip(std::string tripid) {
		this->tracked_trips.erase(tripid);
		this->tracked_trips_arrival_delays.erase(tripid);
		this->tracked_trips_depature_delays.erase(tripid);
	}

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

		auto compareTripUpdates = [&](std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &old, std::vector<std::shared_ptr<GtfsTripTimeUpdate>> &current, std::string tripid = "") {

#define LOG_TRIPSTATUS(arguments, ...) printf(("NYCTFeedTracker: \33[1;37m'%s'\33[0m: " arguments), tripid.c_str(), ##__VA_ARGS__)

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

			int all_delta_arrival_time = INT_MAX;
			int all_delta_depature_time = INT_MAX;

			int first_arrival_diff_if_schedule_change = 0;
			int first_depature_diff_if_schedule_change = 0;

			int must_print_diffs = 0;

			for (auto i : current_map) {
				auto lT = old_map[i.first];
				if (lT == nullptr) {
					LOG_TRIPSTATUS("NEWDATA for stop %s\n", i.first.c_str());
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

					//printf("NYCTFeedTracker: TRIPCHANGE (for stop #%s)", i.first.c_str());

					if ((lT->arrival_time != rT->arrival_time) && lT->arrival_time && rT->arrival_time) {
						long long arrival_diff = rT->arrival_time - lT->arrival_time;
						if (all_delta_arrival_time == INT_MAX) {
							all_delta_arrival_time = arrival_diff;
						} else if (all_delta_arrival_time != arrival_diff) {
							all_delta_arrival_time = INT_MIN;
							must_print_diffs = 0xC0fee;
							LOG_TRIPSTATUS(CH_MAGENTA "Trip encountered schedule change starting from arrival for stop '%s'.\n" CRESET, i.first.c_str());
							first_arrival_diff_if_schedule_change = arrival_diff;
						}
					}

					if ((lT->depature_time != rT->depature_time) && lT->depature_time && rT->depature_time) {
						long long depature_diff = rT->depature_time - lT->depature_time;
						if (all_delta_depature_time == INT_MAX) {
							all_delta_depature_time = depature_diff;
						} else if (all_delta_depature_time != depature_diff) {
							all_delta_depature_time = INT_MIN;
							must_print_diffs = 0xC0fee;
							LOG_TRIPSTATUS(CH_MAGENTA "Trip encountered schedule change starting from depature for stop '%s'.\n" CRESET, i.first.c_str());
							first_depature_diff_if_schedule_change = depature_diff;
						}
					}

					if (lT->scheduled_track != rT->scheduled_track) {
						LOG_TRIPSTATUS("TRACKUPDATE Scheduled track for trip stop '%s' has changed!\n", i.first.c_str());
						must_print_diffs |= 1;
					}

					if (lT->actual_track != rT->actual_track) {
						if (lT->actual_track == "") {
							LOG_TRIPSTATUS("TRACKUPDATE Now have expected track data for trip stop '%s'\n", i.first.c_str());
							must_print_diffs |= 1;
						} else if (rT->actual_track == "") {
							LOG_TRIPSTATUS(CMAGENTA "LOSTDATA Somehow lost actual track data for stop '%s'\n" CRESET, i.first.c_str());
							must_print_diffs |= 1;
						} else {
							LOG_TRIPSTATUS(CH_YELLOW "REROUTE Train is no longer expected to stop at stop '%s' at track '%s', will now stop at track '%s'\n" CRESET,
								i.first.c_str(), lT->actual_track.c_str(), rT->actual_track.c_str());
						}
					}

					// am i not fucking clever for this trickery ?!
					if (must_print_diffs > 0) {
						printf("\nwas: ");
						printTripTimeData(*lT);
						printf("\nnow: ");
						printTripTimeData(*rT);
						printf("\n");
						must_print_diffs--;
					}
				}
				old_map.erase(i.first);
			}

			//assert(all_delta_arrival_time != INT_MIN);
			//assert(all_delta_depature_time != INT_MIN);

			bool should_print_first_tripupdate_diff = false;

			std::stringstream trip_arrivals_string;
			std::stringstream trip_depatures_string;
			std::stringstream trip_cumulative_string;

			if (all_delta_arrival_time == INT_MAX) {
				//there was no difference in data (this is ok)
				//printf("NYCTFeedTracker: No stop arrival time updates for trip?\n");
			} else if (all_delta_arrival_time != INT_MIN) {
				this->tracked_trips_arrival_delays[tripid] += all_delta_arrival_time;
				if (all_delta_arrival_time > 0)
					trip_arrivals_string << CH_YELLOW "Trip arrivals are now " << all_delta_arrival_time << " seconds delayed. " CRESET;
				else if (all_delta_arrival_time < 0)
					trip_arrivals_string << CYELLOW "Trip arrivals are now " << -all_delta_arrival_time <<  " seconds early. " CRESET;
				should_print_first_tripupdate_diff = true;
				auto cumulative_arrival_delay = this->tracked_trips_arrival_delays[tripid];
				if (cumulative_arrival_delay != all_delta_arrival_time) {
					trip_cumulative_string << CH_YELLOW "Cumulative arrival delay for trip is now " << cumulative_arrival_delay << "  seconds. " CRESET;
				}
			} else /* all_delta_arrival_time == INT_MIN */{
				this->tracked_trips_arrival_delays[tripid] += first_arrival_diff_if_schedule_change;
				auto cumdiff = this->tracked_trips_arrival_delays[tripid];
				trip_cumulative_string << CH_YELLOW "Trip encountered a arrival schedule change, only accounting for first difference - Cumulative arrival delay for trip is now " << cumdiff << "  seconds. " CRESET;
			}

			if (all_delta_depature_time == INT_MAX) {
				//there was no difference in data (this is ok)
			} else if (all_delta_depature_time != INT_MIN) {
				this->tracked_trips_depature_delays[tripid] += all_delta_depature_time;
				if (all_delta_depature_time > 0)
					trip_depatures_string << CH_YELLOW "Trip depatures are now " << all_delta_depature_time << " seconds delayed. " CRESET;
				else if (all_delta_arrival_time < 0)
					trip_depatures_string << CYELLOW "Trip depatures are now " << -all_delta_depature_time << " seconds early. " CRESET;
				should_print_first_tripupdate_diff = true;

				auto cumulative_depature_delay = this->tracked_trips_depature_delays[tripid];
				if (cumulative_depature_delay != all_delta_depature_time) {
					trip_cumulative_string << CH_YELLOW "Cumulative depature delay for trip is now " << cumulative_depature_delay << "  seconds. " CRESET;
				}
			} else /* all_delta_depature_time == INT_MIN */ {
				this->tracked_trips_depature_delays[tripid] += first_depature_diff_if_schedule_change;
				auto cumdiff = this->tracked_trips_depature_delays[tripid];
				trip_cumulative_string << CH_YELLOW "Trip encountered a depature schedule change, only accounting for first difference - Cumulative depature delay for trip is now " << cumdiff << "  seconds. " CRESET;
			}

			// Below only gets called if all arrivals/depatures change by a fixed amount, so we only print
			// the difference once instead of them all. If arrivals/depatures change in a non-fixed manner,
			// then print all the differences.
			if (should_print_first_tripupdate_diff) {
				LOG_TRIPSTATUS("%s %s\n", trip_arrivals_string.str().c_str(), trip_depatures_string.str().c_str());
				if (trip_cumulative_string.tellp())
					LOG_TRIPSTATUS("%s\n", trip_cumulative_string.str().c_str());
				printf("was: ");
				printTripTimeData(*((NYCTTripTimeUpdate*)old[0].get()));
				printf("\nnow: ");
				printTripTimeData(*((NYCTTripTimeUpdate*)current[0].get()));
				printf("\n");
			}

			for (auto i : old_map) {
				printf("NYCTFeedTracker: LOSTTRIPTIME '%s' - train probably passed this stop.\n", tripid.c_str());
				printTripTimeData(*i.second);
				printf("\n");
			}
		};

		currentFeed->forEachVehicleUpdate([&](NYCTVehicleUpdate *vu) {
			NYCTTrip *trip = (NYCTTrip*)vu->trip.get();

			if (this->tracked_vehicles.find(trip->nyct_train_id) == this->tracked_vehicles.end()) {
				this->tracked_vehicles[trip->nyct_train_id] = NYCTVehicleUpdate(*vu);
			} else /* already exists */ {
				auto current = this->tracked_vehicles[trip->nyct_train_id];
				if (current.current_stop_index != vu->current_stop_index) {
					printf("NYCTFeedTracker: Train with ID '%s' is now at stop #%d, was at stop #%d\n", trip->nyct_train_id.c_str(), vu->current_stop_index, current.current_stop_index);
				}
				this->tracked_vehicles[trip->nyct_train_id] = NYCTVehicleUpdate(*vu);
			}
		});

		// Every vehicle feed above will also have a TripUpdate, however not every TU will have a VU (e.g. special non-revenue trips)
		// TODO: IF the above is *not* true, we will leak memory -- look into this
		currentFeed->forEachTripUpdate([&](NYCTTripUpdate *tu) {
			NYCTTrip *trip = (NYCTTrip*)tu->trip.get();
			std::string trainid = trip->nyct_train_id;

			if (!trip->nyct_is_assigned && tu->stop_time_updates.size() == 0) {
				printf("NYCTTrainTracker: " CPURPLE "Not tracking an unassigned trip with no stop time updates with ID '%s'.\n" CRESET, trainid.c_str());
				return;
			}
			
			if (/*this->tracked_ticker [don't remember what this is] */true) {
				if (this->tracked_trips.find(trainid) == this->tracked_trips.end()) {
					printf("NYCTTrainTracker: " CH_RED "New untracked train with ID '%s'" CRESET, trainid.c_str());
					if (this->tracked_vehicles.find(trainid) != this->tracked_vehicles.end()) {
						printf(" (at stop #%d)", this->tracked_vehicles[trainid].current_stop_index);
					}
					if (trainid.at(0) != '0') {
						printf(CH_MAGENTA " (Non-Revenue");
						switch (trainid.at(0)) {
						case '=':
							printf(" SERVICE REROUTE");
							break;
						case '/':
							printf(" SKIP-STOP");
							break;
						case '$':
							printf(" TURN TRAIN");
							break;
						default:
							printf(" unknown trip purpose");
						}
						printf(")" CRESET);
					}
					if (!trip->nyct_is_assigned)
						printf(" (UNASSIGNED)\n");
					else
						printf("\n");
					this->tracked_trips[trainid] = NYCTTripUpdate(*tu);
				} else {
					NYCTTrip *oldtrip = (NYCTTrip*)tracked_trips[trainid].trip.get();
					//TODO
					//printf("NYCTTrainT")
					if (!oldtrip->nyct_is_assigned && trip->nyct_is_assigned) {
						printf("NYCTTrainTracker: " CGREEN "Formerly unassigned trip with ID '%s' is now assigned.\n" CRESET, trainid.c_str());
					} else if (oldtrip->nyct_is_assigned && !trip->nyct_is_assigned) {
						printf("NYCTTrainTracker: " CYELLOW "Trip id '%s' is now unassigned?!\n" CRESET, trainid.c_str());
					}
					compareTripUpdates(tracked_trips[trainid].stop_time_updates, tu->stop_time_updates, trainid);
					unaccounted_trips.erase(trainid);
					this->tracked_trips[trainid] = NYCTTripUpdate(*tu);
					// the above is a very questionable line because it means we loose all the
					// initial data for a trip -- perhaps we may want to store this data and changes
					// until the trip is complete.
				}
			}
		});

		for (auto unaccounted_trip : unaccounted_trips) {
			printf("NYCTTrainTracker: " CH_GREEN "Lost track of trip '%s'" CRESET "\n", unaccounted_trip.first.c_str());
			//unaccounted_trip.second.stop_time_updates.at(0)->stop_id;
			this->clearTrackedDataForTrip(unaccounted_trip.first);
		}
		return true;
	}

	void NYCTFeedTracker::run() {
		while (this->active) {
			bool ret = this->update();
			if (!ret) {
				SLEEP(1000);
				this->update(); // try once more if we fail first time
								// this may happen if the server goes breifly offline

			}
			//todo: sleep until 20 seconds after the lastest feed update relative to last TS
			// MTA updates their feeds every 15 seconds
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
