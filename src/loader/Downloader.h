#pragma once

#include "happyhttp\happyhttp.h"
#include <string>

#define BUFFER_SIZE 524288

class Downloader {
public:
	Downloader(std::string file, std::string tmp) : file(file), buffer(new char[BUFFER_SIZE]), etag("0"),
		conn(new happyhttp::Connection("client.haloanticheat.com", 80)) { };
	~Downloader();
	void Go();
	bool Pump();
	void* GetBuffer();
	void setEtag(char* etag);

private:
	std::string etag;
	std::string file;
	std::string tmp;
	void DecryptFile();
	happyhttp::Connection* conn;
	void* buffer;
};