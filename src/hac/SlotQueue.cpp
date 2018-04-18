#include "SlotQueue.h"
#include "QueryParser.h"
#include "DisplayUpdater.h"
#include "EnginePointers.h"
#include "EngineFunctions.h"
#include "EngineState.h"
#include "EngineTypes.h"
#include "ServerQuery.h"
#include "DebugHelper.h"
#include "console.h"
#include "Shared.h"
#include "ServerInformation.h"
#include "Language.h"
#include "EventDispatcher.h"
#include <sstream>
#include <Windows.h>
#include <process.h>
#include <memory>
#include <string>

using namespace EngineTypes;
using namespace EngineState;

namespace {
	void  __cdecl queuePlayer(void* info);
	bool isFull(const char* response);
	bool hideSplash;
	bool joinQueue(const ServerInfo& info);
}

bool fullQueue(const char* response, const ServerInfo& info, bool _hideSplash) {
	setState(QUEUED);
	clearState(LEAVE_QUEUE);

	hideSplash = _hideSplash;

	if(isFull(response) && joinQueue(info)) {
		return true;
	}

	clearState(QUEUED);

	return false;
}

bool fullQueue(const char* response, const ConnectionDetails* details) {
    ServerInfo info(details);
	return fullQueue(response, info, false);
}

namespace {

bool joinQueue(const ServerInfo& info) {
    ServerInfo* serverInfo = new ServerInfo(info);

	if(_beginthread(queuePlayer, 0, (void*)serverInfo) != -1L) {
		return true;
	} else {
		hkDrawText(Language::GetString(GENERIC_ERROR), C_TEXT_RED);	
		delete serverInfo;
		return false;
	}
}

bool isFull(const char* response) {
	QueryParser parser(response);
	int maxPlayers = atoi(parser.getValue("maxplayers").c_str());
	int numPlayers = atoi(parser.getValue("numplayers").c_str());
	return numPlayers >= maxPlayers;
}

void __cdecl queuePlayer(void* info) {
	dispatcher->enqueue(std::make_shared<Event>(QUEUE_JOIN));
	ServerInfo* serverInfo = static_cast<ServerInfo*>(info);
	ServerQuery query(serverInfo->ip.c_str(), serverInfo->port);
	char queryBuffer[4096] = {};
	bool doConnect = false, runQuery = true;
	int failedQueries = 0;
	short minutes = 0, ticks = 0;
	double seconds = 0;
	std::string message = Language::GetString(SERVER_FULL);

	//Set UI up
	char buffer[128] = {};
	sprintf_s(buffer, 128, message.c_str(), minutes, seconds);

	DisplayUpdater splash;
	splash.showMessage(buffer);
	
	if(!hideSplash) {
		splash.begin();
	}
	
	//Set timer up
	LARGE_INTEGER freq, begin, end;
	QueryPerformanceFrequency(&freq);
    double frequency = DWORD(freq.QuadPart) / 1000.0;

	while(!checkState(LEAVE_QUEUE)) {
		QueryPerformanceCounter(&begin);

		//Run query
		if(runQuery) {
			if(query.query(queryBuffer, 4096)) {
				if(!isFull(queryBuffer)) {
					doConnect = true;
					break;
				}
				failedQueries = 0;
			} else {
				failedQueries++;
			}
			runQuery = false;
		}

		if(failedQueries >= 3) {
			splash.showMessage(Language::GetString(SERVER_CLOSED));
			Sleep(2000);
			break;
		}

		sprintf_s(buffer, 64, message.c_str(), minutes, floor(seconds));

		splash.showMessage(buffer);
		
		Sleep(200);
		QueryPerformanceCounter(&end);
		seconds += (DWORD(end.QuadPart - begin.QuadPart) / frequency) / 1000;
		
		if(seconds >= 60) {
			minutes++;
		}

		ticks++;

		if(ticks > 10) {
			runQuery = true;
			ticks = 0;
		}

		seconds = fmod(seconds, 60);
	}

	clearState(QUEUED);

	if(doConnect) {
		if(hideSplash) {
			commandConnect(serverInfo->ip, serverInfo->port, serverInfo->password);
		} else {
			commandConnect(serverInfo->ip, serverInfo->port, reconnectPassword);
		}
	}

	dispatcher->enqueue(std::make_shared<Event>(QUEUE_LEAVE));
	delete serverInfo;
}

}