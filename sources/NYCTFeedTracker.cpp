#include "NYCTFeedTracker.h"

#include <time.h>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
// Sleep for a given number of milliseconds
#define SLEEP Sleep
#else
#include <unistd.h>
// Sleep for a given number of milliseconds
#define SLEEP(x) usleep(x * 1000)
#endif

#include "Logging.h"

namespace nyctlib {
	void NYCTFeedTracker::clearTrackedDataForTrip(std::string tripid) {
		this->tracked_trips2.erase(tripid);
	}

	void NYCTFeedTracker::processTripTimeUpdates(std::string tripid, std::vector<std::shared_ptr<GtfsTripTimeUpdate>>& old, std::vector<std::shared_ptr<GtfsTripTimeUpdate>>& current) {

#define LOG_TRIPSTATUS_WARN(arguments, ...) LOG_FT_WARN("NYCTFeedTracker: \33[1;37m'%s'\33[0m: " arguments, tripid.c_str(), ##__VA_ARGS__)
#define LOG_TRIPSTATUS_DEBUG(arguments, ...) LOG_FT_DEBUG("NYCTFeedTracker: \33[1;37m'%s'\33[0m: " arguments, tripid.c_str(), ##__VA_ARGS__)

		auto printTripTimeData = [](const NYCTTripTimeUpdate& l) {
			LOG_FT_DEBUG("stop_id=%s, arrival_time=%lld, depature_time=%lld, scheduled_track=%s, actual_track=%s",
				l.stop_id.c_str(), l.arrival_time, l.depature_time, l.scheduled_track.c_str(), l.actual_track.c_str());
		};

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
				LOG_TRIPSTATUS_DEBUG("NEWDATA for stop %s\n", i.first.c_str());
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
						LOG_TRIPSTATUS_WARN(CH_MAGENTA "Trip encountered schedule change starting from arrival for stop '%s'.\n" CRESET, i.first.c_str());
						first_arrival_diff_if_schedule_change = arrival_diff;
					}
				}

				if ((lT->depature_time != rT->depature_time) && lT->depature_time && rT->depature_time) {
					long long depature_diff = rT->depature_time - lT->depature_time;
					if (all_delta_depature_time == INT_MAX) {
						all_delta_depature_time = depature_diff;
					}
					else if (all_delta_depature_time != depature_diff) {
						all_delta_depature_time = INT_MIN;
						must_print_diffs = 0xC0fee;
						LOG_TRIPSTATUS_WARN(CH_MAGENTA "Trip encountered schedule change starting from depature for stop '%s'.\n" CRESET, i.first.c_str());
						first_depature_diff_if_schedule_change = depature_diff;
					}
				}

				if (lT->scheduled_track != rT->scheduled_track) {
					LOG_TRIPSTATUS_WARN("TRACKUPDATE Scheduled track for trip stop '%s' has changed!\n", i.first.c_str());
					must_print_diffs |= 1;
				}

				if (lT->actual_track != rT->actual_track) {
					if (lT->actual_track == "") {
						LOG_TRIPSTATUS_DEBUG("TRACKUPDATE Now have expected track data for trip stop '%s'\n", i.first.c_str());
						must_print_diffs |= 1;
					} else if (rT->actual_track == "") {
						LOG_TRIPSTATUS_WARN(CMAGENTA "LOSTDATA Somehow lost actual track data for stop '%s'\n" CRESET, i.first.c_str());
						must_print_diffs |= 1;
					} else {
						LOG_TRIPSTATUS_WARN(CH_YELLOW "REROUTE Train is no longer expected to stop at stop '%s' at track '%s', will now stop at track '%s'\n" CRESET,
							i.first.c_str(), lT->actual_track.c_str(), rT->actual_track.c_str());
					}
				}

				// am i not fucking clever for this trickery ?!
				if (must_print_diffs > 0) {
					LOG_RAW_DEBUG("was: ");
					printTripTimeData(*lT);
					LOG_RAW_DEBUG("\nnow: ");
					printTripTimeData(*rT);
					LOG_RAW_DEBUG("\n");
					must_print_diffs--;
				}
			}
			old_map.erase(i.first);
		}

		bool should_print_first_tripupdate_diff = false;

		std::stringstream trip_arrivals_string;
		std::stringstream trip_depatures_string;
		std::stringstream trip_cumulative_string;

		if (all_delta_arrival_time == INT_MAX) {
			//there was no difference in data (this is ok)
			//printf("NYCTFeedTracker: No stop arrival time updates for trip?\n");
		} else if (all_delta_arrival_time != INT_MIN) {
			this->tracked_trips2[tripid].cumulative_arrival_delay += all_delta_arrival_time;
			if (all_delta_arrival_time > 0)
				trip_arrivals_string << CH_YELLOW "Trip arrivals are now " << all_delta_arrival_time << " seconds delayed. " CRESET;
			else if (all_delta_arrival_time < 0)
				trip_arrivals_string << CYELLOW "Trip arrivals are now " << -all_delta_arrival_time << " seconds early. " CRESET;
			should_print_first_tripupdate_diff = true;
			auto cumulative_arrival_delay = this->tracked_trips2[tripid].cumulative_arrival_delay;
			if (cumulative_arrival_delay != all_delta_arrival_time) {
				trip_cumulative_string << CH_YELLOW "Cumulative arrival delay for trip is now " << cumulative_arrival_delay << "  seconds. " CRESET;
			}
		} else /* all_delta_arrival_time == INT_MIN */ {
			this->tracked_trips2[tripid].cumulative_arrival_delay += first_arrival_diff_if_schedule_change;
			auto cumdiff = this->tracked_trips2[tripid].cumulative_arrival_delay;
			trip_cumulative_string << CH_YELLOW "Trip encountered a arrival schedule change, only accounting for first difference - Cumulative arrival delay for trip is now " << cumdiff << "  seconds. " CRESET;
		}

		if (all_delta_depature_time == INT_MAX) {
			//there was no difference in data (this is ok)
		} else if (all_delta_depature_time != INT_MIN) {
			this->tracked_trips2[tripid].cumulative_depature_delay += all_delta_depature_time;
			if (all_delta_depature_time > 0)
				trip_depatures_string << CH_YELLOW "Trip depatures are now " << all_delta_depature_time << " seconds delayed. " CRESET;
			else if (all_delta_arrival_time < 0)
				trip_depatures_string << CYELLOW "Trip depatures are now " << -all_delta_depature_time << " seconds early. " CRESET;
			should_print_first_tripupdate_diff = true;

			auto cumulative_depature_delay = this->tracked_trips2[tripid].cumulative_depature_delay;
			if (cumulative_depature_delay != all_delta_depature_time) {
				trip_cumulative_string << CH_YELLOW "Cumulative depature delay for trip is now " << cumulative_depature_delay << "  seconds. " CRESET;
			}
		} else /* all_delta_depature_time == INT_MIN */ {
			this->tracked_trips2[tripid].cumulative_depature_delay += first_depature_diff_if_schedule_change;
			auto cumdiff = this->tracked_trips2[tripid].cumulative_depature_delay;
			trip_cumulative_string << CH_YELLOW "Trip encountered a depature schedule change, only accounting for first difference - Cumulative depature delay for trip is now " << cumdiff << "  seconds. " CRESET;
		}

		// Below only gets called if all arrivals/depatures change by a fixed amount, so we only print
		// the difference once instead of them all. If arrivals/depatures change in a non-fixed manner,
		// then print all the differences.
		if (should_print_first_tripupdate_diff) {
			LOG_TRIPSTATUS_WARN("%s %s\n", trip_arrivals_string.str().c_str(), trip_depatures_string.str().c_str());
			if (trip_cumulative_string.tellp())
				LOG_TRIPSTATUS_WARN("%s\n", trip_cumulative_string.str().c_str());
			LOG_RAW_DEBUG("was: ");
			printTripTimeData(*((NYCTTripTimeUpdate*)old[0].get()));
			LOG_RAW_DEBUG("\nnow: ");
			printTripTimeData(*((NYCTTripTimeUpdate*)current[0].get()));
			LOG_RAW_DEBUG("\n");
		}

		for (auto i : old_map) {
			LOG_FT_INFO("NYCTFeedTracker: LOSTTRIPTIME '%s' - train probably passed this stop.\n", tripid.c_str());
			printTripTimeData(*i.second);
			LOG_RAW_DEBUG("\n");
		}
	}

	int NYCTFeedTracker::getStopIndexRelativeToInitialSchedule(std::string trip_id, std::string gtfs_stop_id) {
		assert(this->tracked_trips2.find(trip_id) != this->tracked_trips2.end());
		auto is = this->tracked_trips2[trip_id].initial_trip_schedule;
		if (is.size() == 0) {
			LOG_FT_DEBUG("Cannot return stop index for trip '%s' without any assigned schedule!\n", trip_id.c_str());
			return -1;
		}
		
		for (int i = 0; i < is.size(); i++) {
			auto s = is[i];
			if (s.stop_id == gtfs_stop_id) {
				return i + 1; // trip id indexes always start at 1
			}
		}

		return 0;
	}

	bool NYCTFeedTracker::update() {
		feed->update();

		auto currentFeed = feed->getCurrentFeed();
		
		if (currentFeed == nullptr) {
			printf("Cannot update because currentFeed was nullptr.\n");
			return false;
		}

		if (currentFeed->getFeedTime() == 0) {
			printf("Cannot accept currentFeed because its timestamp was 0.\n");
			return false;
		}

		//this->last_tracked_trips.clear();
		//this->last_tracked_vehicles.clear();

		auto unaccounted_trips = this->tracked_trips2;

		currentFeed->forEachVehicleUpdate([&](NYCTVehicleUpdate *vu) {
			NYCTTrip *trip = (NYCTTrip*)vu->trip.get();

			// The following below must never be true -- if it is, we probably have bad data!!
			// assert(trip->nyct_train_id != "");

			auto tripid = trip->nyct_train_id;

			if (tripid == "") {
				LOG_FT_WARN(CH_RED "Invalid Vehicle Update: NYCT Trip ID must never be null.\n");
				return;
			}

			if (this->tracked_trips2.find(tripid) == this->tracked_trips2.end()) {
				this->tracked_trips2[tripid].last_tracked_vehicle = NYCTVehicleUpdate(*vu);
			} else {
				auto &tracked = this->tracked_trips2[tripid];
				auto &tracked_trip = tracked.last_tracked_vehicle;

				auto original_trip_schedule = tracked.initial_trip_schedule;
				auto old_stop_progress = tracked_trip.stop_progress;
				auto current_stop_progress = vu->stop_progress;

				if (current_stop_progress == GtfsVehicleProgress::AtStation) {
					bool found_trip = false;
					for (auto schedule_item : original_trip_schedule) {
						if (schedule_item.stop_id == vu->stop_id) {
							found_trip = true;
						}
					}

					if (found_trip) {
						LOG_FT_DEBUG("Trip '%s' at stop '%s' as per schedule.\n",
							trip->nyct_train_id.c_str(), vu->stop_id.c_str());
						tracked.confirmed_stops.push_back(std::make_tuple(vu->stop_id, vu->timestamp));
					} else {
						LOG_FT_WARN(CH_YELLOW "Trip '%s' is stopping at stop '%s', was not on the original schedule.\n" CRESET,
							trip->nyct_train_id.c_str(), vu->stop_id.c_str());
						tracked.confirmed_stops.push_back(std::make_tuple(vu->stop_id, vu->timestamp));
					}
				}

				auto getStopProgressString = [](GtfsVehicleProgress progress) {
					if (progress == GtfsVehicleProgress::ApproachingStation) {
						return "approaching";
					}
					else if (progress == GtfsVehicleProgress::AtStation) {
						return "at";
					}
					else if (progress == GtfsVehicleProgress::EnrouteToStation) {
						return "enroute to";
					}
					return "unknown";
				};

				if (tracked_trip.current_stop_index != vu->current_stop_index) {
					LOG_FT_DEBUG("NYCTFeedTracker: Trip '%s' is now %s stop #%d, was previously %s stop #%d\n", trip->nyct_train_id.c_str(), getStopProgressString(current_stop_progress), vu->current_stop_index, getStopProgressString(old_stop_progress), tracked_trip.current_stop_index);
				}

				if (vu->current_stop_index == 1) {
					LOG_FT_DEBUG("Trip '%s' is at its first stop!\n", tripid.c_str());
				}

				tracked.last_tracked_vehicle = NYCTVehicleUpdate(*vu);
			}
		});

		// Every vehicle feed above will also have a TripUpdate, however not every TU will have a VU (e.g. special non-revenue trips)
		// TODO: IF the above is *not* true, we will leak memory -- look into this
		currentFeed->forEachTripUpdate([&](NYCTTripUpdate *tu) {
			NYCTTrip *trip = (NYCTTrip*)tu->trip.get();
			std::string trainid = trip->nyct_train_id;

			if (!trip->nyct_is_assigned && tu->stop_time_updates.size() == 0) {
				LOG_FT_INFO("NYCTTrainTracker: " CPURPLE "Not tracking an unassigned trip with no stop time updates with ID '%s'.\n" CRESET, trainid.c_str());
				return;
			}

			bool no_vechicle_info = false;

			if (this->tracked_trips2.find(trainid) == this->tracked_trips2.end()) {
				// New Train Without Any Vehicle Data
				no_vechicle_info = true;
			}
			
			{
				auto &tracked_trip = this->tracked_trips2[trainid]; // created if not exists

				if (!tracked_trip.last_tracked_trip) {
					LOG_FT_INFO("NYCTTrainTracker: " CH_GREEN "New untracked train with ID '%s'" CRESET, trainid.c_str());


					if (tracked_trip.last_tracked_vehicle) {
						LOG_RAW_INFO(" (at stop #%d/%d)", tracked_trip.last_tracked_vehicle.current_stop_index, tu->stop_time_updates.size());
					}

					if (trainid.at(0) != '0') {
						LOG_RAW_INFO(CH_MAGENTA " (Non-Revenue");
						switch (trainid.at(0)) {
						case '=':
							LOG_RAW_INFO(" SERVICE REROUTE");
							break;
						case '/':
							LOG_RAW_INFO(" SKIP-STOP");
							break;
						case '$':
							LOG_RAW_INFO(" TURN TRAIN");
							break;
						default:
							LOG_RAW_INFO(" unknown trip purpose");
						}
						LOG_RAW_INFO(")" CRESET);
					}

					if (!trip->nyct_is_assigned) {
						LOG_RAW_INFO(" (UNASSIGNED)\n");
					} else {
						LOG_RAW_INFO("\n");
						for (auto stu : tu->stop_time_updates) {
							auto s = (NYCTTripTimeUpdate*)stu.get();
							tracked_trip.initial_trip_schedule.push_back(NYCTTripTimeUpdate(*s));
						}
					}
					tracked_trip.last_tracked_trip = NYCTTripUpdate(*tu);
				} else {
					tracked_trip.old_tracked_trip = tracked_trip.last_tracked_trip;

					NYCTTrip *oldtrip = (NYCTTrip*)tracked_trip.last_tracked_trip.trip.get();

					if (tracked_trip.initial_trip_schedule.size() == 0 && tu->stop_time_updates.size() > 0) {
						LOG_FT_DEBUG("NYCTTrainTracker: Now have initial trip data for trip ID '%s'\n", trainid.c_str());
						for (auto stu : tu->stop_time_updates) {
							auto s = (NYCTTripTimeUpdate*)stu.get();
							tracked_trip.initial_trip_schedule.push_back(NYCTTripTimeUpdate(*s));
						}
					}

					if (!oldtrip->nyct_is_assigned && trip->nyct_is_assigned) {
						LOG_FT_INFO("NYCTTrainTracker: " CGREEN "Formerly unassigned trip with ID '%s' is now assigned. " CRESET "(at stop #%d/#%d)\n",
							trainid.c_str(), tracked_trip.last_tracked_vehicle.current_stop_index, tu->stop_time_updates.size());
						if (tracked_trip.initial_trip_schedule.size() == 0) {
							LOG_FT_WARN("NYCTrainTracker: " CYELLOW "Trip ID '%s' was assigned without any stop time updates\n" CRESET, trainid.c_str());
						}
					} else if (oldtrip->nyct_is_assigned && !trip->nyct_is_assigned) {
						LOG_FT_WARN("NYCTTrainTracker: " CH_MAGENTA "Trip id '%s' is now unassigned?!\n" CRESET, trainid.c_str());
					}

					this->processTripTimeUpdates(trainid, tracked_trip.last_tracked_trip.stop_time_updates, tu->stop_time_updates);
					unaccounted_trips.erase(trainid);
					tracked_trip.last_tracked_trip = NYCTTripUpdate(*tu);
				}
			}
		});

		// if we loose track of more than 10 trips within 45 sec, something probably went wrong
		// unless 10 trains got to their terminals in less than a minute. more likely we got bad data.
		auto time_diff_from_last = currentFeed->getFeedTime() - this->last_update_time;
		if (unaccounted_trips.size() > 10 && time_diff_from_last <= 45) {
			LOG_FT_WARN("NYCTFeedTracker: " CH_RED "Too many trains are now unaccounted for -- refusing to drop tracking: %d > 10\n" CRESET, unaccounted_trips.size());
			return false;
		}

		for (auto unaccounted_trip : unaccounted_trips) {
			auto tripid = unaccounted_trip.first;
			auto tracked = unaccounted_trip.second;
			if (tracked.last_tracked_vehicle) {
				auto tracked_vech = tracked.last_tracked_vehicle;
				auto current_stop_index = tracked_vech.current_stop_index;
				auto scheduled_stop_count = tracked.initial_trip_schedule.size();
				auto last_accounted_stop_id = tracked_vech.stop_id;
				auto its = tracked.initial_trip_schedule;
				auto scheduled_terminal_stop_id = its[(int)its.size() - 1].stop_id;

				if (last_accounted_stop_id == scheduled_terminal_stop_id) {
					LOG_FT_INFO("NYCTrainTracker: " CH_BLUE "Trip '%s' is complete and has reached its scheduled terminal.\n" CRESET, tripid.c_str());
				} else {
					LOG_FT_INFO("NYCTTrainTracker: " CH_RED "Lost track of trip '%s'" CRESET "\n", tripid.c_str());
				}

				LOG_FT_DEBUG("NYCTTrainTracker: Index was %d/%d (%s/%s)\n",
					current_stop_index,
					scheduled_stop_count,
					last_accounted_stop_id.c_str(),
					scheduled_terminal_stop_id.c_str());
			}

			this->clearTrackedDataForTrip(unaccounted_trip.first);
		}

		this->last_update_time = currentFeed->getFeedTime();
		return true;
	}

	void NYCTFeedTracker::run() {
		while (this->active) {
			bool ret = this->update();
			if (!ret) {
				SLEEP(1500);
				this->update(); // try once more if we fail first time
								// this may happen if the server goes breifly offline
			}
			
			auto next_update = this->last_update_time + 24;
			long long timenow = time(NULL);
			auto sleep_for_seconds = next_update - timenow;
			sleep_for_seconds = max(1, sleep_for_seconds);
			sleep_for_seconds = min(15, sleep_for_seconds);
			// above is range function to ensure seconds is in [1,15]

			// MTA updates their feeds every 15 seconds, we check back the data every 24 seconds
			// which appears to be a good wait period to allow the server to update + serve data
			printf("done, going back to sleep for %d seconds. zzz.\n", sleep_for_seconds);
			SLEEP(sleep_for_seconds * 1000);
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
