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
		std::vector<NYCTFeedTracker*> trackers;
	public:
		volatile bool running;

		NYCTSubwayInterface(WSInterface *ws_interface, std::vector<NYCTFeedTracker*> trackers,
			std::shared_ptr<BlockingEventHolder<SubwayTripEvent>> event_holder);

		void processRequest(WSInterface::Client client, std::string request, std::string paramaters);

		void processFlatRequest(WSInterface::Client client, char *message, int message_len);

		void respond(WSInterface::Client client, std::string data) {
			if (client != nullptr) {
				//this->wsInterface->respondCommand(client, "subway", "");
			}
		}

		json11::Json jBuildStopMessageUpdate(SubwayTripEvent &e);

		json11::Json jBuildTripScheduleUpdate(SubwayTripEvent &e);

		void fBuildStopMessageUpdate(SubwayTripEvent &e, unsigned char* &message, int &message_len);

		void fBuildTripScheduleUpdate(SubwayTripEvent &e, unsigned char* &message, int &message_len);

		void run();

		void pGeneralFeedSubscribeRequest(WSInterface::Client client) {
			this->gf_subscribed_clients.push_back(client);
		}

		void pLatestTripUpdateRequest(WSInterface::Client client, std::string trip_id);

		void pCurrentTripsRequest(WSInterface::Client client);
	};
}