#include "ServerInformation.h"
#include "EngineTypes.h"
#include <WinSock2.h>

ServerInfo::ServerInfo(const EngineTypes::ConnectionDetails* details) {
	in_addr address;
	address.s_addr = htonl((long)details->ip);
	ip = inet_ntoa(address);
	port = details->port;
}