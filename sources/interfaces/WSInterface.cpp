#include <iostream>
#include <ctime>
#include <cassert>
#include <uWS/uWS.h>
#include "interfaces/WSInterface.h"
#include "Globals.h"
#include "rtnyc_protocol_generated.h"

// TODO: Remove this later on, currently used for debugging a uWS bug
#define LOG_DEBUG printf

#define GETSERVER(client) (uWS::WebSocket<uWS::SERVER>*)client
#define GETGROUP(client) ((uWS::Group<uWS::SERVER>*)client)

using namespace json11;

namespace nyctlib {

	void createClientIdFromSockAddr() {

	}

	Json WSInterface::createJsonRequestMessage() {
		Json::object obj{
			{"version", this->version},
			{"type", 1},
			{"time", (int)time(NULL)}, // I'm sure this tool won't be around by the time this is a problem! (lookup '2038 problem')
		};

		return obj;
	}

	void WSInterface::createJsonResponseHead(json11::Json::object &object, MessageType message_type) {
		object["v"] = 1;
		object["ts"] = (int)time(NULL);
		object["t"] = message_type;
	}

	void WSInterface::processTextMessage(Client client, char *message, size_t length) {
		uWS::WebSocket<uWS::SERVER> *ws = GETSERVER(client);
		std::string msg(message, length);
		std::cout << "WSI: <- '" << msg << "'" << std::endl;

		std::string error;

		auto data = Json::parse(msg.c_str(), error);

		if (data.is_null()) {
			printf("WSI: processTextMessage(): Dropped Message - Failed to parse JSON: %s\n", error.c_str());
			return;
		}

		if (this->jrecieve_handler)
			this->jrecieve_handler(ws, data);

		auto _version = data["v"];
		auto _timestamp = data["ts"];
		auto _message_type = data["t"];

		assert(_version.is_number());
		assert(_version.int_value() == 1);

		auto message_type = _message_type.int_value();

		if (message_type == 1) {
			auto request_body = data["request"];

			if (request_body.is_null()) {
				Json::object json;
				createJsonResponseHead(json, MessageType::Error);
				json["message"] = "No request body";
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
				createJsonResponseHead(json, MessageType::Error);
				json["message"] = "Invalid command service";
				json["ts"] = (int)time(NULL);
				Json j(json);
				ws->send(j.dump().c_str());
				return;
			}
		} else if (message_type == 3) { // SetPreferredMessageFormat
			auto _requested_format = data["format"];
			auto requested_format = _requested_format.int_value();

			if (requested_format == 1) { // JSON
				this->client_map[client].requested_format = FormatJson;
				// No need to increment has_json_clients as this is already the default
			} else if (requested_format == 2) { // Binary Flatbuffers
				if (this->client_map[client].requested_format != FormatFlatbuffer) {
					// User wants to switch from JSON to binary
					this->client_map[client].requested_format = FormatFlatbuffer;
					this->has_json_clients--;
					this->has_flat_clients++;
				}
			}

			this->respondStatus(client, "OK", MessageType::Response);
		} else if (message_type == 334) {
			/*std::thread([this]() {
				Sleep(1000);
				this->terminate();
			}).detach();*/
			
			this->stop(1001, "Client requested server terminate");
		}
	}

	void WSInterface::processBinaryMessage(Client client, char *message, size_t length) {
		uWS::WebSocket<uWS::SERVER> *ws = GETSERVER(client);

		if (length < 8) {
			LOG_DEBUG("WSI: INVALID Binary input message from '%s'\n", ws->getAddress().address);
		}

		short version = 1;
		memcpy(&length, &version, 2);

		int flatlen = 0;
		memcpy(&flatlen, &message[2], 4);

		int message_type;
		memcpy(&message_type, &message[6], 4);

		LOG_DEBUG("WSI: Cannot currently handle message with v%d, len=%d, message_type=%d\n",
			version, flatlen, message_type);

		if (message_type == ClientCommandProxyRequest) {
			// TODO: Implement.
		}
	}

	void WSInterface::start(std::string bind_host, short bind_port) {
		auto listener = new uWS::Hub();
		this->server = listener;

		listener->onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
			
			if (opCode == uWS::OpCode::TEXT) {
				processTextMessage(ws, message, length);
			} else if (opCode == uWS::OpCode::BINARY) {
				processBinaryMessage(ws, message, length);
			} else {
				LOG_DEBUG("WSI: Got unknown opCode, cannot accept message: %d\n", opCode);
			}
			
		});

		listener->onError([](int port) {
			std::cout << "Could not bind to port " << port << "!" << std::endl;
			exit(-1);
		});
		//listener->terminate();
		
		listener->onConnection([&](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
			
			printf("WSI: New connection from address '%s:%d'\n", ws->getAddress().address, ws->getAddress().port);
			
			// Let the code below remain as a visible reminisance of how much C++ is a PITA
			//ws->terminate();
			//((uWS::Group<uWS::SERVER>*)((uWS::Hub*)listener))->terminate();
			//printf("listener=%d, server=%d\n", listener, server);
			//((uWS::Group<uWS::SERVER>*)(uWS::Hub*)server)->terminate();
			//this->terminate();
			
			this->has_json_clients++;
			this->client_map[ws] = ClientDetails{ time(NULL), FormatJson };
		});

		listener->onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *msg, size_t len) {
			printf("WSI: Client '%s:%d' disconnected, code %d %s\n",
				ws->getAddress().address, ws->getAddress().port, code, std::string(msg, len).c_str());
			auto client = this->client_map[ws];
			if (client.requested_format == FormatJson)
				this->has_json_clients--;
			if (client.requested_format == FormatFlatbuffer)
				this->has_flat_clients--;
			this->client_map.erase(ws);
		});


		if (listener->listen("0.0.0.0", bind_port)) {
			printf("WSI: Now listening on '%s:%d'\n", bind_host.c_str(), bind_port);
			listener->run();
		}

		printf("WSI: Ended.\n");
	}

	// For some ongodly reason the program will crash if you don't
	// do this double pointer casting. I have no fucking idea why.
	// It took hours to hunt this issue down.
	void WSInterface::stop(int code, std::string message) {
		((uWS::Group<uWS::SERVER>*)(uWS::Hub*)server)->close(code, (char*)message.data(), message.length());
	}

	void WSInterface::terminate() {
		((uWS::Group<uWS::SERVER>*)(uWS::Hub*)server)->terminate();
	}

	void WSInterface::respondCommand(Client client, std::string service, std::string command, Json &data) {
		json11::Json::object json;
		createJsonResponseHead(json);
		json["response"] = Json::object{
			{"f", service},
			{"c", command},
			{"data", data}
		};
		json["ts"] = (int)time(NULL);
		auto wsclient = (uWS::WebSocket<uWS::SERVER>*)client;
		Json j(json);
		wsclient->send(j.dump().c_str());
	}

	void WSInterface::broadcastJsonPreferred(std::string service,
		std::vector<std::pair<std::string /* command */, json11::Json /* data */>> &updates) {
		
		json11::Json::object json;
		createJsonResponseHead(json, MessageType::SubscribtionFeed);
		
		Json::array bupdates;
		for (auto update : updates) {
			bupdates.push_back(Json::object{
				{ "f", service },
				{ "c", update.first },
				{ "data", update.second }
			});
		}
		
		json["updates"] = bupdates;
		json["ts"] = (int)time(NULL);

		Json j(json);
		std::string message = j.dump();
		auto prepared = uWS::WebSocket<uWS::SERVER>::prepareMessage((char*)message.c_str(), message.length(), uWS::TEXT, false);

		LOG_DEBUG("WSI: Broadcasting to JSON %d clients\n", client_map.size());
		for (auto user : client_map) {
			if (user.second.requested_format != FormatJson)
				continue;

			auto ws = (uWS::WebSocket<uWS::SERVER>*)user.first;
			LOG_DEBUG("WSI: Broadcasting to FD%d (shuttingdown=%d, remBytes=%d)\n", 
				ws->getFd(), ws->isShuttingDown(), ws->remainingBytes);
#ifdef _WIN32
			// Fixes bug with sending multiple messages one after the other
			// That would result in program faults
			Sleep(100);
#endif
			if (!ws->isClosed()) {
				ws->sendPrepared(prepared);
			}	
		}
	}

	void WSInterface::broadcastBinaryPreferred(int message_type, char *message, int message_len) {
		LOG_DEBUG("WSI: Writing %d bytes with message_type=%d", message_len, message_type);
		char *ws_message = new char[message_len + 14 /* message_len int, message_type int */];
		short version = 1;
		memcpy(ws_message, &version, 2);
		memcpy(ws_message + 2, &message_len, 4);
		memcpy(ws_message + 6, &message_type, 4);
		memcpy(ws_message + 10, message, message_len);
		memset(ws_message + message_len + 10, 0, 4); // zero term

		auto prepared = uWS::WebSocket<uWS::SERVER>::prepareMessage(ws_message, message_len + 10, uWS::BINARY, false);
		for (auto client : client_map) {
			if (client.second.requested_format != FormatFlatbuffer)
				continue;
			auto ws = GETSERVER(client.first);
			ws->sendPrepared(prepared);
		}
		delete[] ws_message;
	}

	void WSInterface::broadcastBinaryPreferredBatch(std::vector<BinaryMessageWrapper> messages) {
		int current_index = 0;
		int messages_len = 0;
		for (auto &message : messages) {
			messages_len += message.message_len + 8;
		}
		LOG_DEBUG("WSI: Writing %d batch messages -- len=%d\n", messages.size(), messages_len);


		char *ws_message = new char[messages_len + 6 /* 2 byte, end 4 byte padding */];

		short version = 1;
		memcpy(ws_message, &version, 2);
		current_index += 2;

		for (auto message : messages) {
			memcpy(ws_message + current_index, &message.message_len, 4); 
			current_index += 4;
			memcpy(ws_message + current_index, &message.message_type, 4);
			current_index += 4;
			memcpy(ws_message + current_index, message.message, message.message_len);
			current_index += message.message_len;
			delete[] message.message;
		}

		memset(ws_message + current_index, 0, 4); // zero term
		
		//auto prepared = uWS::WebSocket<uWS::SERVER>::prepareMessage(ws_message, messages_len + 6, uWS::BINARY, false);
		for (auto client : client_map) {
			if (client.second.requested_format != FormatFlatbuffer)
				continue;
			auto ws = GETSERVER(client.first);
			ws->send(ws_message, messages_len + 6, uWS::BINARY, nullptr, nullptr, true);
			//ws->sendPrepared(prepared);
		}

		delete[] ws_message;
	}

	void WSInterface::respondError(Client client, std::string error_message) {
		Json::object json;
		createJsonResponseHead(json, MessageType::Error);
		json["message"] = error_message;
		Json j(json);
		auto wsclient = (uWS::WebSocket<uWS::SERVER>*)client;
		wsclient->send(j.dump().c_str());
		return;
	}

	void WSInterface::respondStatus(Client client, std::string message, MessageType message_type) {
		Json::object json;
		createJsonResponseHead(json, message_type);
		json["message"] = message;
		json["ts"] = (int)time(NULL);
		Json j(json);
		auto wsclient = (uWS::WebSocket<uWS::SERVER>*)client;
		wsclient->send(j.dump().c_str());
		return;
	}
}