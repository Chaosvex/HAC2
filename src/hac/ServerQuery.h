#pragma once

#include <string>

class ServerInfo;

class ServerQuery {
	long tSecs, tMicros;
	std::string ip;
	unsigned short port;
	char* response;
	std::string parseQueryString(char* response, const std::string& key);

public:
	ServerQuery(const char* ip, unsigned short port, long tSeconds = 2, long tMicros = 0);
	ServerQuery(const ServerInfo& info, long tSeconds = 2, long tMicros = 0);
	~ServerQuery() {};
	bool query(char* buffer, int buffLen);
};