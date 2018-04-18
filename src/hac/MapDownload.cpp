#include "MapDownload.h"
#include "DisplayUpdater.h"
#include "PatchGroup.h"
#include "EngineTypes.h"
#include "EngineState.h"
#include "MapChecksum.h"
#include "misc.h"
#include "QueryParser.h"
#include "base64.h"
#include "URLEncode.h"
#include "json\json.h"
#include "happyhttp\happyhttp.h"
#include "xzip\XUnzip.h"
#include "console.h"
#include "EngineFunctions.h"
#include "MapControl.h"
#include "Shared.h"
#include "EnginePointers.h"
#include "Language.h"
#include "ServerInformation.h"
#include "File.h"
#include <Windows.h>
#include <sstream>
#include <process.h>
#include <memory>

using namespace happyhttp;
using namespace EngineTypes;
using namespace EngineState;

struct RequestResponse;

std::unique_ptr<RequestResponse> parseResponse(const std::string& json);

struct DownloadArgs {
	DownloadArgs(std::string map, DisplayUpdater& splash, const File& file) : handle(file), success(false), mapFound(true),
		splash(splash), speed(0), done(0), total(0), bytesRecv(0),
		prevBytes(0) {};
	std::string map;
	const char* response;
	DisplayUpdater& splash;
	int speed, done, total,
		bytesRecv, prevBytes;
	bool success, mapFound;
	const File& handle;
};

enum MIRROR_TYPE {
	ACTIVE_MIRROR,
	STORAGE_ONLY
};

struct ControlThreadArgs {
	HANDLE patchSem;
	const char* response;
};

struct RequestResponse {
	std::uint32_t status;
	std::string message;
	std::uint32_t checksum;
	std::string hostname;
	std::string uri;
	std::string token;
	std::string map;
	MIRROR_TYPE type;
};

void progressMessage(DownloadArgs* args, const char* cLength) {
	static int lastCheck = 0;
	DWORD currTime = GetTickCount();

	//Measure speed every half second - not super accurate
	if((currTime - lastCheck) > 500) {
		int deltaKBytes = (args->bytesRecv - args->prevBytes) / 1024;
		deltaKBytes *= 2;
		args->speed = deltaKBytes;
		args->prevBytes = args->bytesRecv;
		lastCheck = currTime;
	}

	int length = atoi(cLength);
	float totalMB = (length / 1024.f) / 1024.f;
	float recvMB = (args->bytesRecv / 1024.f) / 1024.f;

	args->splash.updateProgress(recvMB, totalMB, args->speed);
}

/*************************************************
 *         CALLBACKS FOR MAP DOWNLOADING         *
 ************************************************/

void OnBegin(const happyhttp::Response* r, void* userdata) {
	DownloadArgs* args = static_cast<DownloadArgs*>(userdata);
	if(r->getstatus() == 404) {
		args->mapFound = false;
	} else if(r->getstatus() >= 200 && r->getstatus() < 300) {
		args->splash.showMessage(Language::GetString(DOWNLOAD_FOUND));
		args->mapFound = true;
	} else {
		args->splash.showMessage(Language::GetString(DOWNLOAD_HTTP_ERROR));
		args->mapFound = false;
	}
}

void OnData(const happyhttp::Response* r, void* userdata, const unsigned char* data, int n) {
	DownloadArgs* args = static_cast<DownloadArgs*>(userdata);
	fwrite(data, n, 1, args->handle);
	args->bytesRecv += n;
	progressMessage(args, r->getheader("Content-length"));
}

/*************************************************
 *           CALLBACKS FOR URL FETCHING          *
 ************************************************/

void OnFetchData(const happyhttp::Response* r, void* userdata, const unsigned char* data, int n) {
	std::string* buffer = (std::string*)userdata;
	buffer->append(reinterpret_cast<char*>(const_cast<unsigned char*>(data)), n);
}

/*
 * Extracts a map file from a downloaded archive.
 * Note: Should pass in the archive name too.
 */
void extractMap(const std::string& map) {
	std::string file_name = map + ".map";
	std::string extraction_path = getWorkingDir() + "maps/" + map + ".map";
	std::string tmp_path = getWorkingDir() + "tmp.map";

	HZIP zipHandle = OpenZip((void*)tmp_path.c_str(), 0, ZIP_FILENAME);

	if(zipHandle == NULL) {
		throw MapDownloadException(Language::GetString(DOWNLOAD_ARCHIVE_DAMAGED));
	}

	int index = 0;
	ZRESULT res = FindZipItem(zipHandle, file_name.c_str(), true, &index, NULL);

	if(res != ZR_OK) {
		CloseZip(zipHandle);
		throw MapDownloadException(Language::GetString(DOWNLOAD_FILE_MISSING));
	}

	ZRESULT result = UnzipItem(zipHandle, index, (void*)extraction_path.c_str(), 0, ZIP_FILENAME);

	if(result != ZR_OK) {
		CloseZip(zipHandle);
		throw MapDownloadException(Language::GetString(DOWNLOAD_DECOMPRESS_ERROR));
	}

	CloseZip(zipHandle);
}

/*
 * Fetches the map's checksum, adds it to the hash-map and then requests
 * Halo to add it to map list.
 */
void installMap(const std::string& map, uint32_t checksum) {
	if(checksum == 0) {
		std::string path = getWorkingDir() + "maps/" + map + ".map";
		MapChecksum checksummer(path);
		checksum = checksummer.checksum();
	}
	
	addChecksum(map, checksum);
	addMap(map.c_str());
}

std::unique_ptr<RequestResponse> fetchMirrorDetails(const std::string& map) {
	char* key = "test"; //temp
	char transformed[64] = {};
	std::string buffer;

	for(size_t i = 0, j = strlen(key); i < map.length(); ++i) {
		transformed[i] = map[i] ^ key[i % j];
	}

	std::string encoded = urlencode(base64_encode(reinterpret_cast<const unsigned char*>(transformed), map.size()));

	std::string uri("/request/map/");
	uri += encoded;
	happyhttp::Connection conn("maps.haloanticheat.com", 80);
	conn.setcallbacks(NULL, OnFetchData, NULL, &buffer);
	conn.request("GET", uri.c_str(), 0, 0, 0);

	while(conn.outstanding()) {
		conn.pump();
	}

	return parseResponse(buffer);
}

std::unique_ptr<RequestResponse> parseResponse(const std::string& json) {
	std::unique_ptr<RequestResponse> resp(new RequestResponse());
	Json::Value values;
	Json::Reader reader;

	try {
		reader.parse(json, values);
		resp->checksum = values.get("checksum", 0).asUInt();
		resp->type = static_cast<MIRROR_TYPE>(values.get("type", 0).asInt());
		resp->status = values.get("status", 0).asInt();
		resp->message = values.get("message", "").asString();
		resp->token = values.get("token", "").asString();
		resp->hostname = values.get("hostname", "").asString();
		resp->uri = values.get("uri", "").asString();
		resp->map = values.get("map", "").asString();
	} catch(std::exception& e) {
		resp->status = -1;
		resp->message = "Error: ";
		resp->message += e.what();
	}

	return resp;
}

/*
 * Begins a map download.
 */
uint32_t fetchMap(DisplayUpdater& splash, const std::string& map, const File& file) {
	DownloadArgs args(map, splash, file);
	
	//Fetch download mirror URL
	std::unique_ptr<RequestResponse> mResponse = fetchMirrorDetails(map);
	
	if(mResponse->status != 200) {
		throw MapDownloadException(mResponse->message);
	}

	//Begin download
	if(mResponse->type == 1) {
		mResponse->uri += urlencode(mResponse->map);
	}

	const char* headers[] = {"Content-type", "application/x-www-form-urlencoded", 0};
	std::string body = "token=" + mResponse->token;
	
	happyhttp::Connection conn(mResponse->hostname.c_str(), 80);
	conn.setcallbacks(OnBegin, OnData, NULL, &args);
	conn.request("POST", mResponse->uri.c_str(), headers, reinterpret_cast<const unsigned char*>(body.c_str()), body.length());

	while(conn.outstanding() && args.mapFound && !checkState(HALT_DOWNLOAD)) {
		conn.pump();
	}
	
	conn.close();

	if(!args.mapFound) {
		throw MapDownloadException(Language::GetString(DOWNLOAD_FILE_REMOTE_MISSING));
	} else if(checkState(HALT_DOWNLOAD)) {
		throw MapDownloadException(Language::GetString(DOWNLOAD_ABORT));
	}

	return mResponse->checksum;
}

void displayError(const DisplayUpdater& splash, const std::string& message) {
	splash.showMessage(message);
	Sleep(2000);
}

void processRequest(DisplayUpdater& splash, const std::string& map, File& file) {
	uint32_t checksum = fetchMap(splash, map, file);
	splash.showMessage(Language::GetString(DOWNLOAD_CHECKSUMMING));
	file.close();
	extractMap(map);
	installMap(map, checksum);
	splash.showMessage(Language::GetString(DOWNLOAD_SUCCESS));
}

void __cdecl downloadThread(void* arg) {
	ControlThreadArgs* args = static_cast<ControlThreadArgs*>(arg);
	const char* response = args->response;
	
	//Retrieve the name of the map to download
	QueryParser parser(response);
	std::string map = parser.getValue("mapname");

	//Set UI up and begin
	DisplayUpdater splash(map);
	splash.showMessage(Language::GetString(DOWNLOAD_STARTING));
	splash.begin();

	//Signal to main thread that we're done patching the UI
	ReleaseSemaphore(args->patchSem, 1, NULL);

	//Create temporary file
	std::string path = getWorkingDir() + "tmp.map";
	File file(path, "wb");

	if(file != 0) {
		try {		
			processRequest(splash, map, file);
			splash.restore = false;
			clearState(DOWNLOADING); //split second race condition :(
			commandConnect(lastServer.ip, lastServer.port, reconnectPassword);
		} catch(std::runtime_error& e) {
			displayError(splash, e.what());
		}
	} else {
		displayError(splash, Language::GetString(DOWNLOAD_TEMP_ERROR));
	}

	file.close();
	remove(path.c_str()); //clean up temporary file
	splash.unpatch(); //patches need removed prematurely for the fix below to work :(
	
	//Negotiating UI fix
	if(*overlayControl == NEGOTIATING_CONNECTION) {
		Sleep(3000);
		if(*overlayControl == NEGOTIATING_CONNECTION) {
			ShowMainMenu();
			hkDrawText(Language::GetString(CONNECT_ERROR), C_TEXT_RED);
		}
	}

	clearState(DOWNLOADING);
}

void beginDownload(const char* response) {
	setState(DOWNLOADING);
	clearState(HALT_DOWNLOAD);

	ControlThreadArgs args;
	args.patchSem = CreateSemaphore(NULL, 0, 100, NULL);
	args.response = response;

	if(args.patchSem != NULL) {
		HANDLE thread = (HANDLE)_beginthread(downloadThread, 0, &args);
		if(thread != NULL) {
			WaitForSingleObject(args.patchSem, INFINITE);
			CloseHandle(args.patchSem);
		} else {
			hkDrawText(Language::GetString(DOWNLOAD_BEGIN_ERROR), C_TEXT_RED);
			clearState(DOWNLOADING);
		}
	}
}