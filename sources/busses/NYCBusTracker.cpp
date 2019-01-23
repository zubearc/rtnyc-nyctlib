#include "busses/NYCBusTracker.h"
#include "Logging.h"
#include <ctime>

#define LOG_TRACE printf

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

namespace nyctlib {
	void NYCBusTracker::clearTrackedDataForVehicle(std::string vehid) {
		this->tracked_trips.erase(vehid);
	}

	NYCBusTracker::NYCBusTracker(std::unique_ptr<IFeedService<GtfsFeedParser>>& tripUpdateFeed,
		std::unique_ptr<IFeedService<GtfsFeedParser>>& vehicleUpdateFeed, 
		std::shared_ptr<BlockingEventHolder<NYCBusTripEvent>> &event_holder)
		: tripUpdateFeed(std::move(tripUpdateFeed)), 
		vehicleUpdateFeed(std::move(vehicleUpdateFeed)),
		eventHolder(event_holder) {
		
	}

	bool NYCBusTracker::update() {
		tripUpdateFeed->update();
		vehicleUpdateFeed->update();

		auto tuFeed = tripUpdateFeed->getCurrentFeed();
		auto vuFeed = vehicleUpdateFeed->getCurrentFeed();

		auto unaccounted_trips = this->tracked_trips;

		if (vuFeed == nullptr) {
			LOG_TRACE("NYCBusTracker: Cannot update vehicles because vuFeed was nullptr.\n");
		} else if (vuFeed->getFeedTime() == 0) {
			LOG_FT_DEBUG("NYCBusTracker: Cannot accept VU because its timestamp was 0.\n");
		} else {
			vuFeed->forEachVehicleUpdate([&](GtfsVehicleUpdate *vu) {
				//LOG_DEBUG("Handing vehicle update.\n");

				auto trip = vu->trip.get();

				auto vehid = trip->vehicle_id;

				if (this->tracked_trips.find(vehid) == this->tracked_trips.end()) {
					// Below implicitly creates an a tracked_trip.
					this->tracked_trips[vehid].last_tracked_vehicle = GtfsVehicleUpdate(*vu);
					this->tracked_trips[vehid].last_update_time = vuFeed->getFeedTime();
				} else {
					auto &tracked = this->tracked_trips[vehid];
					tracked.last_update_time = vuFeed->getFeedTime();
					auto &tracked_trip = tracked.last_tracked_vehicle;

					auto original_trip_schedule = tracked.initial_trip_schedule;
					auto old_stop_progress = tracked_trip.stop_progress;
					auto current_stop_progress = vu->stop_progress;

					std::string stop_id = vu->stop_id;
					
					auto found_trip = tracked.isStopScheduled(stop_id);

					/*if (current_stop_progress == GtfsVehicleProgress::AtStation) {
						if (found_trip) {
							LOG_FT_DEBUG("Trip '%s' at stop '%s' as per schedule.\n",
								vehid.c_str(), stop_id.c_str());
							tracked.confirmed_stops.push_back(std::make_pair(stop_id, vu->timestamp));
						} else {

						}
					}*/

					auto getStopProgressString = [](GtfsVehicleProgress progress) {
						if (progress == GtfsVehicleProgress::ApproachingStation) {
							return "approaching";
						} else if (progress == GtfsVehicleProgress::AtStation) {
							return "at";
						} else if (progress == GtfsVehicleProgress::EnrouteToStation) {
							return "enroute to";
						}
						return "unknown";
					};

					if (!found_trip) {
						if (tracked.initial_trip_schedule.size())
							LOG_FT_WARN(CH_YELLOW "Trip '%s' is %s stop '%s' (#%d), was not on the original schedule.\n" CRESET,
							vehid.c_str(), getStopProgressString(current_stop_progress), vu->stop_id.c_str(), vu->current_stop_index);
						tracked.confirmed_stops.push_back(std::make_pair(stop_id, vu->timestamp));
					}

					if (tracked_trip.stop_id != vu->stop_id ||
						current_stop_progress != old_stop_progress) {
						LOG_FT_DEBUG("NYCBusTracker: Trip '%s' is now %s stop #%d (%s), was previously %s stop #%d (%s)\n",
							vehid.c_str(), getStopProgressString(current_stop_progress), vu->current_stop_index,
							vu->stop_id.c_str(), getStopProgressString(old_stop_progress), tracked_trip.current_stop_index,
							tracked_trip.stop_id.c_str());

						NYCBusTripEvent event;
						event.trip_id = trip->trip_id;
						event.initial_tracked_trip = &tracked;
						event.event_category = NYCBusTripEvent::StopChange;
						event.vehicle_update = GtfsVehicleUpdate(*vu);
						event.vehicle_update.stop_id = stop_id;
						this->queueEvent(event);
					} else if (tracked_trip.latitude != vu->latitude
						|| tracked_trip.longitude != vu->longitude
						|| tracked_trip.bearing != vu->bearing) {
						NYCBusTripEvent event;
						event.trip_id = trip->trip_id;
						event.initial_tracked_trip = &tracked;
						event.event_category = NYCBusTripEvent::PositionChange;
						event.vehicle_update = GtfsVehicleUpdate(*vu);
						event.vehicle_update.stop_id = stop_id;
						this->queueEvent(event);
					}

					if (vu->current_stop_index == 1) {
						LOG_FT_DEBUG("Trip '%s' is at its first stop!\n", vehid.c_str());
					}

					tracked.last_tracked_vehicle = GtfsVehicleUpdate(*vu);
				}

				std::string action;

				switch (vu->stop_progress) {
				case GtfsVehicleProgress::ApproachingStation:
					action = "approaching";
					break;
				case GtfsVehicleProgress::AtStation:
					action = "at";
					break;
				case GtfsVehicleProgress::EnrouteToStation:
					action = "enroute to";
					break;
				}

				LOG_FT_DEBUG("[%s] trip '%s' (vech '%s') is now %s stop %s\n", vu->trip->route_id.c_str(),
					vu->trip->trip_id.c_str(), vehid.c_str(), action.c_str(), vu->stop_id.c_str());
			});
		}

		if (tuFeed == nullptr) {
			LOG_TRACE("NYCBusTracker: Cannot update trips because tuFeed was nullptr.\n");
		} else if (tuFeed->getFeedTime() == 0) {
			LOG_FT_DEBUG("NYCBusTracker: Cannot accept TU because its timestamp was 0.\n");
		} else {
			tuFeed->forEachTripUpdate([&](GtfsTripUpdate *tu) {
				//LOG_FT_DEBUG("Handing trip update.\n");
				auto trip = tu->trip;

				auto vehid = trip->vehicle_id;
				if (!vehid.size()) {
					printf(CMAGENTA "ERROR: Vehicle ID was not found for [%s], using trip ID '%s' instead...\n" CRESET, trip->route_id.c_str(), trip->trip_id.c_str());
					//assert(vehid.size());
					vehid = trip->trip_id;
				}

				bool new_trip = false;
				if (this->tracked_trips.find(vehid) == this->tracked_trips.end()) {
					LOG_FT_DEBUG(CH_GREEN "NYCBusTracker: [%s] Tracking new vehicle '%s' with trip '%s'\n" CRESET, 
						trip->route_id.c_str(), vehid.c_str(), trip->trip_id.c_str());
					new_trip = true;
				}

				{
					auto &tracked_trip = this->tracked_trips[vehid]; // created if not exists

					if (tracked_trip.last_tracked_trip.trip == nullptr) {
						tracked_trip.last_tracked_trip = GtfsTripUpdate(*tu);
					} else {
						//tracked_trip.old_tracked_trip = tracked_trip.last_tracked_trip;
						
						// code here ...
						
						auto oldtrip = tracked_trip.last_tracked_trip.trip;

						if (tracked_trip.initial_trip_schedule.size() == 0 && tu->stop_time_updates.size() > 0) {
							if (!new_trip)
								LOG_FT_DEBUG("NYCBusTracker: Now have initial trip data for [%s] trip ID '%s'\n", trip->route_id.c_str(), vehid.c_str());
							for (auto stu : tu->stop_time_updates) {
								auto s = stu;
								tracked_trip.initial_trip_schedule.push_back(GtfsTripTimeUpdate(*s));
							}
							NYCBusTripEvent event;
							event.event_category = NYCBusTripEvent::ScheduleChange;
							event.initial_tracked_trip = &tracked_trip;
							event.trip_id = vehid;
							event.trip_update = GtfsTripUpdate(*tu);
							this->queueEvent(event);
						}

						tracked_trip.last_tracked_trip = GtfsTripUpdate(*tu);
					}

				}
				

			});
		}

		//this->last_tracked_trips.clear();
		//this->last_tracked_vehicles.clear();

		//auto unaccounted_trips = this->tracked_trips2;

		if (vuFeed != nullptr)
			this->vu_feed_updated_time = vuFeed->getFeedTime();
		if (tuFeed != nullptr)
			this->tu_feed_updated_time = tuFeed->getFeedTime();

		if (vu_feed_updated_time > 0 && tu_feed_updated_time > 0) {
			this->lastUpdatedTime = min(vu_feed_updated_time, tu_feed_updated_time);
		} else {
			this->lastUpdatedTime = max(vu_feed_updated_time, tu_feed_updated_time);
		}
		
		return true;
	}
	
	
	void NYCBusTracker::run(){
		while (this->active) {
			bool ret = this->update();
			if (!ret) {
				SLEEP(1500);
				this->update(); // try once more if we fail first time
								// this may happen if the server goes breifly offline
			}
			flushEvents();

			auto next_update = this->lastUpdatedTime + 24;
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
}