#include "QueryParser.h"
#include <algorithm>

QueryParser::QueryParser(const char* response) {
	this->response = response;
}

/*
 * Basic parser to extract the currently running map from a server
 * query response.
*/
std::string QueryParser::parseQueryString(char* response, const std::string& key) {
	char *token, *nextToken;
	char *delimiter = "\\";
	token = strtok_s(response, delimiter, &nextToken);

	while(token != NULL) {
		if(strcmp(token, key.c_str()) == 0) {
			token = strtok_s(NULL, delimiter, &nextToken);
			std::string value(token);
			transform(value.begin(), value.end(), value.begin(), ::tolower);
			return value;
		}
		token = strtok_s(NULL, delimiter, &nextToken);
	}

	return "";
}

/*
 * strtok modifies the original string so a copy is 
 * passed rather than the original.
 */
std::string QueryParser::getValue(const std::string& key) {
	char* responseCopy = _strdup(this->response);
	std::string value = parseQueryString(responseCopy, key);
	free(responseCopy);
	return value;
}