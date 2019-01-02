#pragma once

#include <string>
#include <map>
#include <functional>
#include "LibInterface.h"
#include "json11.hpp"

namespace nyctlib {

	class WSInterface : LibInterface {
	private:
		// Below holds server Hub
		void* server;
	public:
		typedef void* Client;

		enum RequestedFeedFormat {
			FormatJson,
			FormatFlatbuffer
		};

		struct ClientDetails {
			long long connection_time;
			RequestedFeedFormat requested_format;
		};

		enum MessageType : unsigned short {
			Status = 0,
			Request = 1,
			Response = 2,
			Error = 3,
			SubscribtionFeed = 4,
			SubscribeRequest = 5, // unused
			UnsubscribeRequest = 6 // unused
		};

		// v1
		enum FlatMessageType_v1 {
			ClientCommandProxyRequest = 0x100,
			ClientCommandProxyResponse = 0x101,
			NYCSubway_StopChange = 0x1001,
			NYCSubway_TripAssigned,
			NYCSubway_ScheduleChange,
			NYCSubway_TripComplete,

			NYCSubway_CurrentTripsResponse = 0x1011,

			NYCBus_StopChange = 0x201,
			NYCBus_TripAssigned,
			NYCBus_ScheduleChange,
			NYCBus_PositionChange,

			NYCBus_CurrentTripsResponse = 0x2011
		};

		// Below are used to make sure we don't serialize data
		// for JSON/Flatbufers if we don't have any clients that
		// are requesting data in such formats to save CPU usage
		int has_json_clients = 0;
		int has_flat_clients = 0;
	private:
		const int version = 1;

		json11::Json createJsonRequestMessage();
		std::string createFlatRequestMessage();
		void createJsonResponseHead(json11::Json::object &object, MessageType message_type = MessageType::Response);

		std::map<Client, ClientDetails> client_map;

		std::function<void(Client, json11::Json&)> jrecieve_handler = nullptr;

		std::map<int /* hash */, 
			std::function<void(Client, char* /* message */, int /* message len */)>> request_handlers;

		std::map<std::string /* For Key */, 
			std::function<void(Client, std::string command, std::string arguments)>> command_handler;

		// INTERNAL MESSAGE PROCESSORS
		void processTextMessage(Client ws, char *message, size_t length);
		void processBinaryMessage(Client client, char *message, size_t length);
	public:
		void start(std::string bind_host = "0.0.0.0", short bind_port = 7204);
		void stop(int code, std::string message);
		void terminate();
		// HANDLERS

		void setRecieveHandler(std::function<void(Client, json11::Json&)> json_input_handler) {
			this->jrecieve_handler = json_input_handler;
		}

		void setCommandHandler(std::string service, 
			std::function<void(Client, std::string command, std::string arguments)> handler) {
			this->command_handler[service] = handler;
		}

		// Binary command handler - no header other than version and body
		void setCommandHandler(int message_type,
			std::function<void(Client, char* /* message */, int /* message len */)> handler) {
			this->request_handlers[message_type] = handler;
		}

		void setRecieveHandler(std::function<void(std::string&)> binary_input_handler) {
			
		}

		// END HANDLERS

		void respondCommand(Client client, std::string service, std::string command, json11::Json &data);
		void respondCommand(Client client, std::string service, std::string command, std::string &data);

		void broadcastJsonPreferred(std::string service, 
			std::vector<std::pair<std::string /* command */, json11::Json /* data */>> &updates);

		void broadcastBinaryPreferred(MessageType reason, int message_type, char *message, int message_len);
	
		struct BinaryMessageWrapper { int message_type; unsigned char *message; int message_len; };
		void broadcastBinaryPreferredBatch(MessageType reason, std::vector<BinaryMessageWrapper> messages);

		void respondError(Client client, std::string error_message);

		void respondStatus(Client client, std::string status_message, MessageType message_type = MessageType::Status);

		~WSInterface() {
			delete server;
		}
	};
}