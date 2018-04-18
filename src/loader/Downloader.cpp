#include "Downloader.h"
#include "happyhttp\happyhttp.h"
#include "DebugHelper.h"
#include <Windows.h>
#include <string>
#include <sstream>

using namespace happyhttp;
using namespace std;

char* buffer = NULL;
FILE* handle;
bool httpError = false;
string errorStr;

void OnBegin(const happyhttp::Response* r, void* userdata) {
	if(r->getstatus() == 200) {
		char path[512];
		GetTempPath(512, path);
		strcat_s(path, 512, "hac.tmp");
		if(fopen_s(&handle, path, "wb") == 0) {
			buffer = new char[BUFFER_SIZE];
		} else {
			throw new Wobbly("Unable to open temporary file for writing.");
		}
	} else if(r->getstatus() == 404) {
		httpError = true;
		errorStr = "The downloader was unable to find the HAC update file! This error could be caused your antivirus program.";
	} else if(r->getstatus() != 304) {
		httpError = true;
		stringstream ss;
		ss << "Received an unexpected response (" << r->getstatus() << ", " << r->getreason() << ") from the HAC update server! This error could be caused your antivirus program.";
		errorStr = ss.str().c_str();
	}
}

void OnData(const happyhttp::Response* r, void* userdata, const unsigned char* data, int n) {
	if(httpError) {
		return;
	}

	static int bytesRecv = 0, pos = 0;
	unsigned char key[24] = {14, 255, 101, 12, 1, 241, 34, 6, 86, 56, 195, 3, 212, 145, 109, 100, 98, 19,
					87, 23, 173, 241, 13, 42};
	memcpy_s(buffer, BUFFER_SIZE, data, n);

	for(int i = 0, j = 24; i < n; i++) {
		buffer[i] ^= key[pos];
		pos = (pos + 1) % j;
	}

	bytesRecv += n;
	fwrite(buffer, n, 1, handle);
}

void OnComplete(const happyhttp::Response* r, void* userdata) {
	if(httpError) {
		return;
	}

	if(r->getstatus() == 200) {
		fclose(handle);
	}

	//Save Etag
	FILE* handle;
	char path[512];
	GetTempPath(512, path);
	strcat_s(path, 512, "hac.tag");
	if(fopen_s(&handle, path, "wb") == 0) {
		fwrite(r->getheader("ETag"), strlen(r->getheader("Etag")), 1, handle);
		fclose(handle);
	} else {
		throw new Wobbly("Unable to open temporary file for writing.");
	}
}

void Downloader::Go() {
	const char* headers[] = {"If-None-Match", etag.c_str(), "Connection", "close", 0};
	string uri = "/" + file;
	conn->setcallbacks(OnBegin, OnData, OnComplete, NULL);
	conn->request("GET", uri.c_str(), headers, NULL, NULL);
}

bool Downloader::Pump() {
	while(conn->outstanding() && !httpError) {
		conn->pump();
		return true;
	}
	
	if(httpError) {
		throw Wobbly(errorStr.c_str());
	}
	
	return false;
}

void* Downloader::GetBuffer() {
	return buffer;
}

Downloader::~Downloader() {
	conn->close();
	delete[] buffer;
}

void Downloader::setEtag(char* etag) {
	this->etag = etag;
}