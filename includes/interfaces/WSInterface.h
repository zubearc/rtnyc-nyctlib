#pragma once

#include <string>
#include <map>
#include <functional>
#include "LibInterface.h"
#include "json11.hpp"


namespace nyctlib {

	class WSInterface : LibInterface {
	public:
		typedef void* Client;

		struct ClientDetails {
			long long connection_time;
		};

		enum MessageType : int {
			Status = 0,
			Request = 1,
			Response = 2,
			Error = 3,
			SubscribtionFeed = 4,
			SubscribeRequest = 5, // unused
			UnsubscribeRequest = 6 // unused
		};
	private:
		const int version = 1;
		json11::Json createRequestMessage();

		std::map<Client, ClientDetails> client_map;

		std::function<void(Client, json11::Json&)> jrecieve_handler = nullptr;

		std::map<std::string /* For Key */, std::function<void(Client, std::string command, std::string arguments)>> command_handler;

		void createResponseHead(json11::Json::object &object, MessageType message_type = MessageType::Response);

	public:
		void start(std::string bind_host = "0.0.0.0", short bind_port = 7204);

		void setRecieveHandler(std::function<void(Client, json11::Json&)> json_input_handler) {
			this->jrecieve_handler = json_input_handler;
		}

		void setCommandHandler(std::string service, std::function<void(Client, std::string command, std::string arguments)> handler) {
			this->command_handler[service] = handler;
		}

		void setRecieveHandler(std::function<void(std::string&)> binary_input_handler) {
			
		}

		void respondCommand(Client client, std::string service, std::string command, json11::Json &data);

		void broadcast(std::string service, 
			std::vector<std::pair<std::string /* command */, json11::Json /* data */>> &updates);

		// never used:
		void broadcast(std::vector<
			std::tuple<
			std::tuple<std::string /* service */, std::string /* command */>,
			json11::Json
			>
		>) {};

		void respondError(Client client, std::string error_message);

		void respondStatus(Client client, std::string error_message);
	};
}