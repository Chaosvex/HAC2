#pragma once

#include <string>
#include <cstdint>

namespace EngineTypes {
	struct ConnectionDetails;
}

class ServerInfo {
public:
    ServerInfo() { };
    ServerInfo(const EngineTypes::ConnectionDetails* details);
    ServerInfo(std::string ip, std::uint16_t port)
		: ip(ip), port(port) { }
	std::string ip;
	std::uint16_t port;
	std::wstring password;
};