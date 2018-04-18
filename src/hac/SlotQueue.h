#pragma once

class ServerInfo;

namespace EngineTypes {
	struct ConnectionDetails;
}

bool fullQueue(const char* response, const EngineTypes::ConnectionDetails* info);
bool fullQueue(const char* response, const ServerInfo& info, bool hideSplash);
