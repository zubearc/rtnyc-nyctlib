#include <iostream>
#include <ctime>
#include <cassert>
#include <uWS/uWS.h>
#include "interfaces/WSInterface.h"
#include "Globals.h"

using namespace json11;

namespace nyctlib {

	void createClientIdFromSockAddr() {

	}

	Json WSInterface::createRequestMessage() {
		Json::object obj{
			{"version", this->version},
			{"type", 1},
			{"time", (int)time(NULL)}, // I'm sure this tool won't be around by the time this is a problem! (lookup '2038 problem')
		};

		return obj;
	}

#define CALL_JSON_RECIEVE_HANDLER(args) if (this->jrecieve_handler) this->jrecieve_handler(args);

	void WSInterface::createResponseHead(json11::Json::object &object, MessageType message_type) {
		object["v"] = 1;
		object["ts"] = (int)time(NULL);
		object["t"] = message_type;
	}

	void WSInterface::start(std::string bind_host, short bind_port) {
		uWS::Hub listener;

		listener.onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
			
			if (opCode != uWS::OpCode::TEXT) {
				printf("WSInterface: Cannot accept message with type %d\n", opCode);
			}
			
			std::string msg(message, length);
			std::cout << "New message: '" << msg << "'" << std::endl;

			std::string error;

			auto data = Json::parse(msg.c_str(), error);

			if (data.is_null()) {
				printf("Failed to parse JSON: %s\n", error.c_str());
				return;
				
			} else {
				printf("ok.\n");
			}
			/*auto j = this->createRequestMessage();
			ws->send(j.dump().c_str(), uWS::OpCode::TEXT);*/

			if (this->jrecieve_handler)
				this->jrecieve_handler(ws, data);

			auto _version = data["v"];
			auto _timestamp = data["ts"];
			auto _message_type = data["t"];

			assert(_version.is_number());

			assert(_version.int_value() == 1);

			if (_message_type.int_value() == 1) {
				auto request_body = data["request"];

				if (request_body.is_null()) {
					Json::object json;
					createResponseHead(json, MessageType::Error);
					json["message"] = "No request body specified";
					Json j(json);
					ws->send(j.dump().c_str());
					return;
				}

				auto _request_for = request_body["f"];
				auto _request_command = request_body["c"];
				auto _request_arguments = request_body["a"];

				auto request_for = _request_for.string_value();
				auto request_command = _request_command.string_value();
				auto request_arguments = _request_arguments.string_value();

				printf("Got a new request type: %s\n", request_for.c_str());

				if (command_handler.find(request_for) != command_handler.end()) {
					auto ch = command_handler[request_for];
					ch(ws, request_command, request_arguments);
					return;
				} else {
					Json::object json;
					createResponseHead(json, MessageType::Error);
					json["message"] = "Invalid command service specified.";
					Json j(json);
					ws->send(j.dump().c_str());
					return;
				}
			}
		});

		listener.onError([](int port) {
			std::cout << "Could not bind port " << port << "!" << std::endl;
			exit(-1);
		});

		listener.onConnection([&](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
			printf("New connection from address '%s:%d'\n", ws->getAddress().address, ws->getAddress().port);
			this->client_map[ws] = ClientDetails{ time(NULL) };
		});

		listener.onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *msg, size_t len) {
			printf("Client '%s:%d' disconnected, code %d %s\n",
				ws->getAddress().address, ws->getAddress().port, code, std::string(msg, len).c_str());
			this->client_map.erase(ws);
		});


		if (listener.listen("0.0.0.0", bind_port)) {
			printf("Now listening on '%s:%d'\n", bind_host.c_str(), bind_port);
			listener.run();
		}

		printf("dome.\n");

		//uv_close(0, 0);
	}

	void WSInterface::respondCommand(Client client, std::string service, std::string command, Json &data) {
		json11::Json::object json;
		createResponseHead(json);
		json["response"] = Json::object{
			{"f", service},
			{"c", command},
			{"data", data}
		};
		auto wsclient = (uWS::WebSocket<uWS::SERVER>*)client;
		Json j(json);
		wsclient->send(j.dump().c_str());
	}

	void WSInterface::broadcast(std::string service, 
		std::vector<std::pair<std::string /* command */, json11::Json /* data */>> &updates) {
		
		json11::Json::object json;
		createResponseHead(json, MessageType::SubscribtionFeed);
		
		Json::array bupdates;
		for (auto update : updates) {
			bupdates.push_back(Json::object{
				{ "f", service },
				{ "c", update.first },
				{ "data", update.second }
			});
		}
		
		json["updates"] = bupdates;
		
		Json j(json);
		std::string message = j.dump();
		auto prepared = uWS::WebSocket<uWS::SERVER>::prepareMessage((char*)message.c_str(), message.length(), uWS::TEXT, false);

		for (auto user : client_map) {
			auto ws = (uWS::WebSocket<uWS::SERVER>*)user.first;
			ws->sendPrepared(prepared);
		}
	}

	void WSInterface::respondError(Client client, std::string error_message) {
		Json::object json;
		createResponseHead(json, MessageType::Error);
		json["message"] = error_message;
		Json j(json);
		auto wsclient = (uWS::WebSocket<uWS::SERVER>*)client;
		wsclient->send(j.dump().c_str());
		return;
	}

	void WSInterface::respondStatus(Client client, std::string error_message) {
		Json::object json;
		createResponseHead(json, MessageType::Status);
		json["message"] = error_message;
		Json j(json);
		auto wsclient = (uWS::WebSocket<uWS::SERVER>*)client;
		wsclient->send(j.dump().c_str());
		return;
	}

}