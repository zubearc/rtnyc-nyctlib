#include <time.h>
#include <sstream>
#include "subway/NYCTFeedTracker.h"
#include "Logging.h"
#include "TimeUtil.h"

namespace nyctlib {

	void NYCTFeedTracker::saveTrip(std::string tripid, SubwayTrackedTrip *tracked) {
#ifndef NO_INTERFACES
		assert(tracked->last_tracked_trip);
		//assert(tracked->initial_trip_schedule.size()); // This can get hit by an UNASSIGNED trip

		if (!tracked->last_tracked_trip) {
			LOG_FT_DEBUG("NYCTFeedTracker: saveTrip: Cannot save trip '%s' because it has no last tracked trip data\n", tripid.c_str());
			return;
		} else if (!tracked->initial_trip_schedule.size()) {
			LOG_FT_DEBUG("NYCTFeedTracker: saveTrip: Cannot save trip '%s' because it has no initial trip schedule data\n", tripid.c_str());
			return;
		}

		auto nyct_trip_id = tripid;
		auto timestamp = tracked->last_update_time;
		auto its = tracked->initial_trip_schedule;
		auto ltt = (NYCTTrip*)tracked->last_tracked_trip.trip.get();
		auto direction = ltt->nyct_direction;
		int first_stop_time = 0;
		if (its.size()) {
			first_stop_time = its[0].arrival_time;
			if (!first_stop_time)
				first_stop_time = its[0].depature_time;
		}

		std::map<std::string, std::vector<SubwayTrackedTrip::ConfirmedStop>> m;

		int paths_count = tracked->gtfs_trip_paths.size();
		for (int i = 0; i < paths_count; i++) {
			auto path = tracked->gtfs_trip_paths[i];
			auto path_str = path.first;
			auto path_ts = path.second;
			auto next_path_ts = INT_MAX;
			if ((i + 1) < paths_count) {
				next_path_ts = tracked->gtfs_trip_paths[i + 1].second;
			}
			
			for (auto confirmed_stop : tracked->confirmed_stops) {
				if (confirmed_stop.timestamp >= path_ts) {
					if (confirmed_stop.timestamp < next_path_ts) {
						m[path_str].push_back(confirmed_stop);
						continue;
					}
					break;
				}
				continue;
			}
		}

		SessInterface::commit(nyct_trip_id, last_update_time, 0, m);
#endif
	}

	void NYCTFeedTracker::clearTrackedDataForTrip(std::string tripid) {
		this->saveTrip(tripid, &this->tracked_trips2[tripid]);
		this->tracked_trips2.erase(tripid);
		SubwayTripEvent event;
		event.trip_id = tripid;
		event.event_category = SubwayTripEvent::TripComplete;
		this->queueEvent(event);
	}

	void NYCTFeedTracker::processTripTimeUpdates(std::string tripid, std::vector<std::shared_ptr<GtfsTripTimeUpdate>>& old, std::vector<std::shared_ptr<GtfsTripTimeUpdate>>& current, const NYCTTripUpdate *tu) {

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
						all_delta_arrival_time = (int)arrival_diff;
					} else if (all_delta_arrival_time != arrival_diff) {
						all_delta_arrival_time = INT_MIN;
						must_print_diffs = 0xC0fee;
						LOG_TRIPSTATUS_WARN(CH_MAGENTA "Trip encountered schedule change starting from arrival for stop '%s'.\n" CRESET, i.first.c_str());
						//RAISE_EVENT()
						first_arrival_diff_if_schedule_change = (int)arrival_diff;
					}
				}

				if ((lT->depature_time != rT->depature_time) && lT->depature_time && rT->depature_time) {
					long long depature_diff = rT->depature_time - lT->depature_time;
					if (all_delta_depature_time == INT_MAX) {
						all_delta_depature_time = (int)depature_diff;
					}
					else if (all_delta_depature_time != depature_diff) {
						all_delta_depature_time = INT_MIN;
						must_print_diffs = 0xC0fee;
						LOG_TRIPSTATUS_WARN(CH_MAGENTA "Trip encountered schedule change starting from depature for stop '%s'.\n" CRESET, i.first.c_str());
						first_depature_diff_if_schedule_change = (int)depature_diff;
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

		if (all_delta_arrival_time < INT_MAX || all_delta_depature_time < INT_MAX) {
			std::vector<NYCTTripTimeUpdate> newttus;
			for (auto stu : tu->stop_time_updates) {
				auto ttu = (NYCTTripTimeUpdate*)stu.get();
				newttus.push_back(NYCTTripTimeUpdate(*ttu));
			}
			// don't think we *really* need to record every trip update
			// since especially the IND feeds change all the damn time
			// the first and latest updated should be fine.
			getTrackedTrip(tripid).updated_trip_schedules = { newttus };

			SubwayTripEvent event;
			event.trip_id = tripid;
			event.event_category = SubwayTripEvent::ScheduleChange;
			event.initial_tracked_trip = &getTrackedTrip(tripid);
			event.trip_update = NYCTTripUpdate(*tu);
			this->queueEvent(event);
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

		auto tracked_trip = this->tracked_trips2[tripid];

		for (auto i : old_map) {
			for (auto confirmed_stop : tracked_trip.confirmed_stops) {
				if (confirmed_stop.stop_id == i.first) {
					// train is confirmed to have skipped this stop.
					LOG_TRIPSTATUS_DEBUG("NYCTFeedTracker: Confirmed skipped stop '%s'.\n", i.first.c_str());
					return;
				}
			}
			
			{
				LOG_TRIPSTATUS_WARN("NYCTFeedTracker: LOSTTRIPTIME train probably passed this stop, but we did not record an arrival: '%s'\n", i.first.c_str());
				printTripTimeData(*i.second);
				LOG_RAW_DEBUG("\n");
			}
		}
	}

	int NYCTFeedTracker::getStopIndexRelativeToInitialSchedule(std::string trip_id, std::string gtfs_stop_id) {
		assert(this->tracked_trips2.find(trip_id) != this->tracked_trips2.end());
		auto is = this->tracked_trips2[trip_id].initial_trip_schedule;
		if (is.size() == 0) {
			LOG_FT_DEBUG("Cannot return stop index for trip '%s' without any assigned schedule!\n", trip_id.c_str());
			return -1;
		}
		
		for (unsigned int i = 0; i < is.size(); i++) {
			auto s = is[i];
			if (s.stop_id == gtfs_stop_id) {
				return i + 1; // trip id indexes always start at 1
			}
		}

		return 0;
	}

#define PUSH_CONFIRMED_STOP(confirmed_stop) \
	if (tracked.confirmed_stops.size()) { \
		if (tracked.confirmed_stops[tracked.confirmed_stops.size() - 1].timestamp != confirmed_stop.timestamp) { \
				tracked.confirmed_stops.push_back(confirmed_stop); \
		} \
	} else { \
		tracked.confirmed_stops.push_back(confirmed_stop); \
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
				// Below implicitly creates an a tracked_trip.
				this->tracked_trips2[tripid].last_tracked_vehicle = NYCTVehicleUpdate(*vu);
				this->tracked_trips2[tripid].last_update_time = currentFeed->getFeedTime();
			} else {
				auto &tracked = this->tracked_trips2[tripid];
				tracked.last_update_time = currentFeed->getFeedTime();
				auto &tracked_trip = tracked.last_tracked_vehicle;

				auto original_trip_schedule = tracked.initial_trip_schedule;
				auto old_stop_progress = tracked_trip.stop_progress;
				auto current_stop_progress = vu->stop_progress;

				std::string stop_id;

				if (vu->stop_id.size() == 0) {
					// no stop_id set -- probably on B division feeds
					// thank whatever person who made these feeds for not providing this data to us
					// so we have to fucking make assumptions to make this work.
					// *sigh*. hopefully this is not too expensive of an operation.
					currentFeed->forEachTripUpdate([&](const NYCTTripUpdate &tu) {
						const NYCTTrip *trip = (NYCTTrip*)tu.trip.get();
						std::string trainid = trip->nyct_train_id;
						if (tripid == trainid) {
							if (tu.stop_time_updates.size() > 0) {
								stop_id = tu.stop_time_updates[0]->stop_id;
								LOG_FT_DEBUG("Assumed VU stop_id from latest TU stop time update for trip '%s': %s\n", tripid.c_str(), stop_id.c_str());
								return;
							}
						}
					});
				} else {
					stop_id = vu->stop_id;
				}

				auto found_trip = tracked.isStopScheduled(stop_id);
				

				if (current_stop_progress == GtfsVehicleProgress::AtStation) {
					if (found_trip) {
						LOG_FT_DEBUG("Trip '%s' at stop '%s' as per schedule.\n",
							trip->nyct_train_id.c_str(), stop_id.c_str());
						SubwayTrackedTrip::ConfirmedStop confirmed_stop;
						confirmed_stop.stop_id = stop_id;
						confirmed_stop.timestamp = vu->timestamp;

						auto track_id_actual = found_trip->actual_track;
						auto track_id_scheduled = found_trip->scheduled_track;
						if (track_id_actual.length()) {
							confirmed_stop.arrival_track = track_id_actual;
						} else {
							confirmed_stop.arrival_track = track_id_scheduled;
						}

						PUSH_CONFIRMED_STOP(confirmed_stop);
					} else {
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

				if (!found_trip) {
					LOG_FT_WARN(CH_YELLOW "Trip '%s' is %s stop '%s' (#%d), was not on the original schedule.\n" CRESET,
						trip->nyct_train_id.c_str(), getStopProgressString(current_stop_progress), vu->stop_id.c_str(), vu->current_stop_index);
					SubwayTrackedTrip::ConfirmedStop cs{ stop_id, vu->timestamp };
					PUSH_CONFIRMED_STOP(cs);
				}

				if (tracked_trip.current_stop_index != vu->current_stop_index ||
					current_stop_progress != old_stop_progress) {
					LOG_FT_DEBUG("NYCTFeedTracker: Trip '%s' is now %s stop #%d (%s), was previously %s stop #%d (%s)\n", 
						trip->nyct_train_id.c_str(), getStopProgressString(current_stop_progress), vu->current_stop_index, 
						vu->stop_id.c_str(), getStopProgressString(old_stop_progress), tracked_trip.current_stop_index,
						tracked_trip.stop_id.c_str());
					
					SubwayTripEvent event;
					event.trip_id = trip->nyct_train_id;
					event.initial_tracked_trip = &tracked;
					event.event_category = SubwayTripEvent::StopChange;
					event.vehicle_update = NYCTVehicleUpdate(*vu);
					event.vehicle_update.stop_id = stop_id;
					this->queueEvent(event);
				}

				if (vu->current_stop_index == 1) {
					LOG_FT_DEBUG("Trip '%s' is at its first stop!\n", tripid.c_str());
				}

				tracked.last_tracked_vehicle = NYCTVehicleUpdate(*vu);
			}
		});

		// Every vehicle feed above will also have a TripUpdate, however not every TU will have a VU (e.g. special non-revenue trips)
		// TODO: IF the above is *not* true, we will leak memory -- look into this
		currentFeed->forEachTripUpdate([&](const NYCTTripUpdate &tu) {
			const NYCTTrip *trip = (NYCTTrip*)tu.trip.get();
			std::string trainid = trip->nyct_train_id;
			std::string gtfstripid = trip->trip_id;

			if (!trip->nyct_is_assigned && tu.stop_time_updates.size() == 0) {
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
						LOG_RAW_INFO(" (at stop #%d/%d)", tracked_trip.last_tracked_vehicle.current_stop_index, tu.stop_time_updates.size());
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
						for (auto &stu : tu.stop_time_updates) {
							auto s = (NYCTTripTimeUpdate*)stu.get();
							tracked_trip.initial_trip_schedule.push_back(NYCTTripTimeUpdate(*s));
						}
						SubwayTripEvent event;
						event.event_category = SubwayTripEvent::TripAssigned;
						event.initial_tracked_trip = &tracked_trip;
						event.trip_id = trainid;
						event.trip_update = NYCTTripUpdate(tu);
						this->queueEvent(event);
					}
					tracked_trip.last_tracked_trip = NYCTTripUpdate(tu);
					tracked_trip.gtfs_trip_paths.push_back(std::make_pair(gtfstripid, tu.timestamp));
				} else {
					tracked_trip.old_tracked_trip = tracked_trip.last_tracked_trip;

					NYCTTrip *oldtrip = (NYCTTrip*)tracked_trip.last_tracked_trip.trip.get();

					if (tracked_trip.initial_trip_schedule.size() == 0 && tu.stop_time_updates.size() > 0) {
						LOG_FT_DEBUG("NYCTTrainTracker: Now have initial trip data for trip ID '%s'\n", trainid.c_str());
						for (auto stu : tu.stop_time_updates) {
							auto s = (NYCTTripTimeUpdate*)stu.get();
							tracked_trip.initial_trip_schedule.push_back(NYCTTripTimeUpdate(*s));

						}
						SubwayTripEvent event;
						event.event_category = SubwayTripEvent::ScheduleChange;
						event.initial_tracked_trip = &tracked_trip;
						event.trip_id = trainid;
						event.trip_update = NYCTTripUpdate(tu);
						this->queueEvent(event);
					}

					if (!oldtrip->nyct_is_assigned && trip->nyct_is_assigned) {
						LOG_FT_INFO("NYCTTrainTracker: " CGREEN "Formerly unassigned trip with ID '%s' is now assigned. " CRESET "(at stop #%d/#%d)\n",
							trainid.c_str(), tracked_trip.last_tracked_vehicle.current_stop_index, tu.stop_time_updates.size());
						SubwayTripEvent event;
						event.event_category = SubwayTripEvent::TripAssigned;
						event.initial_tracked_trip = &tracked_trip;
						event.trip_id = trainid;
						event.trip_update = NYCTTripUpdate(tu);
						this->queueEvent(event);
						if (tracked_trip.initial_trip_schedule.size() == 0) {
							LOG_FT_WARN("NYCTrainTracker: " CYELLOW "Trip ID '%s' was assigned without any stop time updates\n" CRESET, trainid.c_str());
						}
					} else if (oldtrip->nyct_is_assigned && !trip->nyct_is_assigned) {
						LOG_FT_WARN("NYCTTrainTracker: " CH_MAGENTA "Trip id '%s' is now unassigned?!\n" CRESET, trainid.c_str());
					}

					if (oldtrip->trip_id != trip->trip_id) {
						LOG_FT_WARN("NYCTrainTracker: " CH_MAGENTA "Trip ID '%s' had path change from '%s' to '%s'\n" CRESET,
							trainid.c_str(), oldtrip->trip_id.c_str(), trip->trip_id.c_str());
						tracked_trip.gtfs_trip_paths.push_back(std::make_pair(gtfstripid, tu.timestamp));
						SubwayTripEvent event;
						event.event_category = SubwayTripEvent::RouteChange;
						event.initial_tracked_trip = &tracked_trip;
						event.trip_id = trainid;
						event.trip_update = NYCTTripUpdate(tu);
						this->queueEvent(event);
					}

					this->processTripTimeUpdates(trainid, tracked_trip.last_tracked_trip.stop_time_updates, 
						((NYCTTripUpdate*)&tu)->stop_time_updates, &tu);
					unaccounted_trips.erase(trainid);
					tracked_trip.last_tracked_trip = NYCTTripUpdate(tu);
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
				if (its.size() == 0) {
					LOG_FT_DEBUG("NYCTTrainTracker: Lost track of trip '%s' with no initial trip schedule\n", tripid.c_str());
					continue;
				}
				auto scheduled_terminal_stop_id = its[(int)its.size() - 1].stop_id;

				if (last_accounted_stop_id == scheduled_terminal_stop_id) {
					LOG_FT_INFO("NYCTrainTracker: " CH_BLUE "Trip '%s' is complete and has reached its scheduled terminal.\n" CRESET, tripid.c_str());
				} else {
					LOG_FT_INFO("NYCTTrainTracker: " CH_RED "Lost track of trip '%s'" CRESET "\n", tripid.c_str());
					SubwayTripEvent event;
					event.event_category = SubwayTripEvent::LostTrip;
					event.initial_tracked_trip = &tracked;
					event.trip_id = tripid;
					this->queueEvent(event);
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
			flushEvents();
			
			auto next_update = this->last_update_time + 24;
			long long timenow = time(NULL);
			auto sleep_for_seconds = next_update - timenow;
			sleep_for_seconds = max(1, sleep_for_seconds);
			sleep_for_seconds = min(15, sleep_for_seconds);
			// above is range function to ensure seconds is in [1,15]

			// MTA updates their feeds every 15 seconds, we check back the data every 24 seconds
			// which appears to be a good wait period to allow the server to update + serve data
			printf("done, going back to sleep for %lld seconds. zzz.\n", sleep_for_seconds);
			SLEEP((int)sleep_for_seconds * 1000);
		}
	}

	std::vector<NYCTTripUpdate> NYCTFeedTracker::getTripsScheduledToArriveAtStop(std::string stop_id) {
		std::vector<NYCTTripUpdate> trips;

		auto currentFeed = feed->getCurrentFeed();

		if (currentFeed == nullptr) {
			printf("Cannot update because currentFeed was nullptr.\n");
			return trips;
		}

		currentFeed->forEachTripUpdate([&](const NYCTTripUpdate &tu) {
			for (auto timeUpdate : tu.stop_time_updates) {
				if (timeUpdate->stop_id == stop_id) {
					trips.push_back(NYCTTripUpdate(tu));
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
