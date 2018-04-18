#pragma once

#include "ServerInformation.h"
#include <vector>
#include <shlobj.h>

#define BOOKMARKS_MAGIC 0x00B7CC22
#ifndef _DEBUG
	#define FILE_NAME "bookmarks.hac"
#else
	#define FILE_NAME "bookmarksd.hac"
#endif

struct ServerBookmark {
	int id;
	char address[32];
	wchar_t name[128];
	wchar_t password[32];
	operator ServerInfo();
};

class Bookmarker {
	enum BOOKMARK_STATUS {
		UNKNOWN, OFFLINE, ONLINE
	};
	struct QueryArgs {
		QueryArgs(Bookmarker* b, char* a, BOOKMARK_STATUS& s, std::string& m)
			: bookmarker(b), address(a), status(s), map(m) {}
		Bookmarker* bookmarker;
		char *address;
		std::string& map;
		BOOKMARK_STATUS& status;
	};
	static std::vector<ServerBookmark> bookmarks;
	void save();
	void load();
	static bool initialised;
	static unsigned int __stdcall query(void* o);
	void output();
	CRITICAL_SECTION outputCS;
	BOOKMARK_STATUS *statuses;
	std::string *maps;
	bool erase(const char* ip);

public:
	Bookmarker();
	~Bookmarker();
	void bookmark(const char* ip, const wchar_t* reconnectPassword, const wchar_t* serverName);
	void display();
	ServerBookmark get(int id);
	bool remove(size_t index);
	bool clipboard(size_t index);
};

void __cdecl bookmarkDisplayAsync(void*);
void bookmarkRemove(BYTE* pKeyboardBuffer);
void bookmarkConnect(BYTE* pKeyboardBuffer);