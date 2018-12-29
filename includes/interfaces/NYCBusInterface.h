#pragma once

#include <atomic>
#include "WSInterface.h"
#include "busses/NYCBusTracker.h"
#include "events/NYCBusTripEvent.h"
#include "events/EventHolder.h"

namespace nyctlib {
	
	class NYCBusInterface {

		std::shared_ptr<BlockingEventHolder<NYCBusTripEvent>> holder;

		WSInterface *wsInterface;
		std::vector<NYCBusTracker*> trackers;

	public:

		volatile bool running = false;

		NYCBusInterface(WSInterface *ws_interface, std::vector<NYCBusTracker*> trackers,
			std::shared_ptr<BlockingEventHolder<NYCBusTripEvent>> event_holder);

		void processRequest(WSInterface::Client client, std::string request, std::string paramaters);

		json11::Json jBuildStopMessageUpdate(NYCBusTripEvent &e);
		json11::Json jBuildTripScheduleUpdate(NYCBusTripEvent &e);

		void fBuildStopMessageUpdate(NYCBusTripEvent &e, unsigned char* &message, int &message_len);
		void fBuildTripScheduleUpdate(NYCBusTripEvent &e, unsigned char* &message, int &message_len);

		void run();

		void pCurrentTripsRequest(WSInterface::Client client);
	};
}