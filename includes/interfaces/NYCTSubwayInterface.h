#pragma once

#include <atomic>
#include "WSInterface.h"
#include "subway/NYCTFeedTracker.h"
#include "events/SubwayTripEvent.h"
#include "events/EventHolder.h"

namespace nyctlib {
	class NYCTSubwayInterface {
	public:
		enum SupportedOperations {
			GetTripStatus,
			GetTripsForRoute,
			GetTripsForStop
		};
	private:

		std::vector<WSInterface::Client> gf_subscribed_clients;

		std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> holder;

		WSInterface *wsInterface;
		NYCTFeedTracker *tracker;
	public:
		volatile bool running;

		NYCTSubwayInterface(WSInterface *ws_interface, NYCTFeedTracker *tracker, 
			std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> event_holder);

		void processRequest(WSInterface::Client client, std::string request, std::string paramaters);

		void respond(WSInterface::Client client, std::string data) {
			if (client != nullptr) {
				//this->wsInterface->respondCommand(client, "subway", "");
			}
		}

		json11::Json jBuildStopMessageUpdate(SubwayTripEvent &e);

		json11::Json jBuildTripScheduleUpdate(SubwayTripEvent &e);

		void run();

		void pGeneralFeedSubscribeRequest(WSInterface::Client client) {
			this->gf_subscribed_clients.push_back(client);
		}

		void pLatestTripUpdateRequest(WSInterface::Client client, std::string trip_id);
	};
}