#pragma once

#include <string>

class QueryParser {
	const char* response;
	std::string parseQueryString(char* response, const std::string& key);

public:
	QueryParser(const char* response);
	std::string getValue(const std::string& key);
};