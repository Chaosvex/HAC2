#pragma once

#include <string>

class MapDownloadException : public std::runtime_error {
public:
	MapDownloadException() : std::runtime_error("Map download failed!") { }
	MapDownloadException(std::string msg) : std::runtime_error(msg) { };
};

void beginDownload(const char* response);