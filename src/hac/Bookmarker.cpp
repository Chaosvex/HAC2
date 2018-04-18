#include "Bookmarker.h"
#include "ServerQuery.h"
#include "InputHandler.h"
#include "KeyMappings.h"
#include "misc.h"
#include "EnginePointers.h"
#include "EngineFunctions.h"
#include "EngineState.h"
#include "console.h"
#include "Shared.h"
#include "QueryParser.h"
#include "ServerInformation.h"
#include "Language.h"
#include "Benchmark.h"
#include "SlotQueue.h"
#include "Preferences.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <process.h>

using namespace keymappings;
using namespace EngineTypes;
using namespace EngineState;

std::vector<ServerBookmark> Bookmarker::bookmarks;
bool Bookmarker::initialised = false;

Bookmarker::Bookmarker() {
	InitializeCriticalSection(&outputCS);

	if(!initialised) {
		load();
	}
}

ServerBookmark::operator ServerInfo() {
	std::string address(address);
	std::string::iterator position = std::find(address.begin(), address.end(), ':');
	std::string ip(address.begin(), position);
	std::string port(position + 1, address.end());
	ServerInfo info(ip, atoi(port.c_str()));
	info.password = password;
	return info;
}

void Bookmarker::bookmark(const char* ip, const wchar_t* reconnectPassword, const wchar_t* serverName) {
	if(strcmp(pCurrentMap, "ui") == 0) {
		HUDMessage(Language::GetString(BOOKMARK_MENU));
	} else if(erase(ip)) {
		HUDMessage(Language::GetString(BOOKMARK_DELETE));
	} else {
		if(bookmarks.size() < 20) {
			ServerBookmark sb;
			strcpy_s(sb.address, 32, ip);
			wcscpy_s(sb.name, 64, serverName);
			wcscpy_s(sb.password, 32, reconnectPassword);
			bookmarks.emplace_back(sb);
			HUDMessage(Language::GetString(BOOKMARK_ADDED));
		} else {
			HUDMessage(Language::GetString(BOOKMARK_LIMIT));
		}
	}

	save();
	PlayMPSound(_multiplayer_sound_countdown_timer_end);
}

bool Bookmarker::erase(const char* ip) {
	std::vector<ServerBookmark>::iterator it = std::find_if(bookmarks.begin(), bookmarks.end(), 
		[&ip](const ServerBookmark& bm) { return strcmp(ip, bm.address) == 0; } );

	if(it != bookmarks.end()) {
		bookmarks.erase(it);
		return true;
	}

	return false;
}

void Bookmarker::display() {
	if(bookmarks.empty()) {
		hkDrawText(Language::GetString(BOOKMARKS_EMPTY), C_TEXT_YELLOW);
		return;
	}

	//Current value is stored otherwise a new bookmark being added
	//could cause an out of bounds access on the below arrays - refactor
	size_t numBookmarks = bookmarks.size();

	statuses = new BOOKMARK_STATUS[numBookmarks]();
	std::vector<HANDLE> threads;
	maps = new std::string[numBookmarks]();

	hkDrawText(Language::GetString(BOOKMARKS_QUERYING), C_TEXT_YELLOW);

	//Begin querying servers and updating output as responses come back
	size_t i = 0;
	for(auto iter = bookmarks.begin(); iter != bookmarks.end()|| i != numBookmarks; ++iter, ++i) {
		QueryArgs* args = new QueryArgs(this, iter->address, statuses[i], maps[i]);
		threads.emplace_back((HANDLE)_beginthreadex(NULL, NULL, query, (void*)args, NULL, NULL));
	}

	DWORD res = WaitForMultipleObjects(numBookmarks, &threads.front(), TRUE, 5000);

	if(res == WAIT_OBJECT_0) {
		output(); //Final output - can't do real-time ouput because 'cls' glitches :(
	} else {
		hkDrawText(Language::GetString(WORD_ERROR), C_TEXT_RED);
	}

	for(auto i = threads.begin(); i != threads.end(); i++) {
		CloseHandle(*i);
	}

	delete[] maps;
	delete[] statuses;
}

//Inefficient - query system needs a redesign
unsigned int __stdcall Bookmarker::query(void* o) {
	QueryArgs *args = static_cast<QueryArgs*>(o);

	//Parse string - refactor class and hooks to take ServerInfo
	std::string wAddress = args->address;
	int index = wAddress.find(":");

	if(index != std::string::npos) {
		char buffer[4096];
		std::string address = wAddress.substr(0, index);
		std::string port = wAddress.substr(index + 1);

		ServerQuery query = ServerQuery(address.c_str(), atoi(port.c_str()), 1, 0);
		Benchmark bm;
		bm.start();
		bool success = query.query(buffer, 4096);
		bm.stop();
	
		//Doesn't account for errors
		if(success) {
			QueryParser parser = QueryParser(buffer);
			std::stringstream ping;
			ping << floor(bm.time()) << "ms"; 
			args->map = parser.getValue("mapname") + ", " + parser.getValue("gametype") + ", "
				+ parser.getValue("numplayers")	+ "/" + parser.getValue("maxplayers") + ", " + ping.str();
			args->status = ONLINE;
		} else {
			args->map = "offline";
			args->status = OFFLINE;
		}
	}

	delete args;
	return 0;
}

void Bookmarker::output() {
	char buff[256];
	int i = 0;
	C_TEXT_COLOUR colour;

	EnterCriticalSection(&outputCS);
	hkDrawText(Language::GetString(BOOKMARK_INSTRUCTION), C_TEXT_YELLOW);
	for(auto iter = bookmarks.begin(); iter != bookmarks.end(); ++iter, ++i) {
		switch(statuses[i]) {
			case ONLINE:
				colour = C_TEXT_GREEN;
				break;
			case OFFLINE:
				colour = C_TEXT_RED;
				break;
			default:
				maps[i] = "unknown";
				colour = C_TEXT_YELLOW;
		}

		sprintf_s(buff, 256, "%d) %ws [%s]", i + 1, iter->name, maps[i].c_str());
		hkDrawText(buff, colour);
	}
	
	//Hacky
	setState(SLOW_OUTPUT);
	Sleep(12000);
	clearState(SLOW_OUTPUT);

	LeaveCriticalSection(&outputCS);
}

bool Bookmarker::remove(size_t index) {
	if(index <= bookmarks.size() && index > 0) {
		bookmarks.erase(bookmarks.begin() + index - 1);
		save();
		return true;
	}
	return false;
}

ServerBookmark Bookmarker::get(int index) {
	return bookmarks.at(index - 1);
}

void Bookmarker::save() {
	ServerBookmark bookmark;
	std::string path = getWorkingDir() + FILE_NAME;
	std::ofstream ofs(path, std::ios::binary);
	
	for(auto iter = bookmarks.begin(); iter != bookmarks.end(); ++iter) {
		strcpy_s(bookmark.address, 32, iter->address);
		memcpy_s(bookmark.name, 128, iter->name, 128);
		wcscpy_s(bookmark.password, 32, iter->password);
		ofs.write((char*)&bookmark, sizeof(ServerBookmark));
	}

	ofs.close();
}

void Bookmarker::load() {
	std::string path = getWorkingDir() + FILE_NAME;
	std::ifstream ifs(path, std::ios::binary);

	//Get file size and rewind
	ifs.seekg(0, std::ios::end);
	std::streamoff length = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	int entries = 0;

	if(length > 0) {
		entries = (int)length / sizeof(ServerBookmark);
	}

	ServerBookmark bookmark;

	for(int i = 0; i < entries; i++) {
		ifs.read((char*)&bookmark, sizeof(ServerBookmark));
		bookmarks.emplace_back(bookmark);
	}

	initialised = true;
}

bool Bookmarker::clipboard(size_t index) {
	if(index <= bookmarks.size() && index > 0) {
		index--;
		std::wstringstream stream;
		stream << "connect " << bookmarks[index].address << " " << bookmarks[index].password;
		const std::wstring& str  = stream.str();
		HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, (str.length() + 1) * sizeof(wchar_t));
		if(hMem != NULL) {
			memcpy(GlobalLock(hMem), str.c_str(), (str.length() + 1) * sizeof(wchar_t));
			GlobalUnlock(hMem);
			OpenClipboard(0);
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hMem);
			CloseClipboard();
		}
		return true;
	}

	return false;
}

Bookmarker::~Bookmarker() {
	DeleteCriticalSection(&outputCS);
}

void  __cdecl bookmarkDisplayAsync(void*) {
	Bookmarker bookmark;
	bookmark.display();
}

void bookmarkConnect(BYTE* pKeyboardBuffer) {
	for(int i = 0, b = 0; i < 10; i++, b = i) {
		if(pKeyboardBuffer[KEY_1 + i] == 1) {
			//If user is holding left ctrl, select bookmarks from 11-20
			if(pKeyboardBuffer[LEFT_CTRL] != 0) {
				b += 10;
			}

			try {
				Bookmarker bookmark;
				ServerBookmark sb = bookmark.get(b + 1);
				bool silentQueue = Preferences::find<bool>("play_during_queue", false);

				if(silentQueue && strcmp(pCurrentMap, "ui") != 0) {
					if(checkState(EngineState::DOWNLOADING) || checkState(EngineState::QUEUED)) {
						HUDMessage(Language::GetString(QUEUE_ALREADY));
						PlayMPSound(_multiplayer_sound_flag_failure);
						break;
					}

					ServerInfo info(sb);
					ServerQuery query(info);
					char response[4096];

					if(!query.query(response, 4096)) {
						HUDMessage(Language::GetString(BOOKMARK_OFFLINE));
						PlayMPSound(_multiplayer_sound_flag_failure);
						break;
					}
					
					QueryParser parser(response);
					int maxPlayers = atoi(parser.getValue("maxplayers").c_str());
					int numPlayers = atoi(parser.getValue("numplayers").c_str());
					bool serverFull = numPlayers >= maxPlayers;
					
					if(serverFull && fullQueue(response, info, true)) {
						HUDMessage(Language::GetString(BOOKMARK_WAITING));
						PlayMPSound(_multiplayer_sound_countdown_timer_end);
						break;
					}
				}
				
				std::string wAddress = sb.address;
				int index = wAddress.find(":");
				if(index != std::string::npos) {
					std::string address = wAddress.substr(0, index);
					std::string port = wAddress.substr(index + 1);
					commandConnect(address, atoi(port.c_str()), sb.password);
				}
			} catch(std::out_of_range) {
				HUDMessage(Language::GetString(BOOKMARK_OUT_RANGE));
				PlayMPSound(_multiplayer_sound_flag_failure);
			}

			break;
		}
	}
}

void bookmarkRemove(BYTE* pKeyboardBuffer) {
	for(int i = 0, b = 0; i < 10; i++, b = i) {
		if(pKeyboardBuffer[KEY_1 + i] == 1) {
			//If user is holding left ctrl, select bookmarks from 11-20
			if(pKeyboardBuffer[RIGHT_CTRL] != 0 || pKeyboardBuffer[LEFT_CTRL] != 0) {
				b += 10;
			}
			
			Bookmarker bookmark;
			if(bookmark.remove(b + 1)) {
				HUDMessage(Language::GetString(BOOKMARK_DELETE));
				PlayMPSound(_multiplayer_sound_countdown_timer_end);
			} else {
				HUDMessage(Language::GetString(BOOKMARK_OUT_RANGE));
				PlayMPSound(_multiplayer_sound_flag_failure);
			}
		}
	}
}