#pragma once

#include <cstdint>

class ServerInfo;
class EventDispatcher;

#define ADDRESS_LENGTH 32
#define PASSWORD_LENGTH 16

struct ServerDetails {
	enum Type {
		STOCK, PHASOR, GANDANUR, OPENSAUCE, SAPP
	};

	Type type;
	std::uint32_t mod_version;
};

extern ServerInfo lastServer;
extern wchar_t reconnectPassword[PASSWORD_LENGTH];
extern char* mapSearchPath;
extern EventDispatcher* dispatcher;
extern float* tickrate;
extern float* gravity;
extern std::int32_t timeleft;
extern ServerDetails serverInfo; // more hacks
