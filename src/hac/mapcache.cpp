#include "mapcache.h"
#include "Shared.h"
#include "InitHooks.h"
#include "EngineTypes.h"
#include "misc.h"
#include "MapControl.h"
#include "EnginePointers.h"
#include <shlobj.h>
#include <Windows.h>
#include <stdio.h>

const std::uint32_t CACHE_MAGIC = 0x00B7CC22;

#ifdef _DEBUG
	#define FILE_NAME "cached.hac"
#else
	#define FILE_NAME "cache.hac"
#endif

const char* currentMap;
std::map<std::string, std::uint32_t> maps;
bool cacheUpdate = false;

struct ChecksumEntry {
	char name[33];
	DWORD checksum;
};

std::uintptr_t DoChecksum;
std::uintptr_t ContinueChecksum;
std::uintptr_t BackToPathLoad;
std::uintptr_t BackToLoading;

/*
 * Writes checksums to the cache file.
 */
void AddChecksum(int checksum) {
	maps[currentMap] = checksum;
	cacheUpdate = true;
}

/*
 * Writes the checksum hash-map to the cache file.
 */
void __stdcall writeCache() {
	FILE* fHandle;
	ChecksumEntry mapChecksum;
	DWORD magic = CACHE_MAGIC;
	std::string path = getWorkingDir();

	#ifdef _DEBUG
		path += "cached.hac";
	#else
		path += "cache.hac";
	#endif

	if(fopen_s(&fHandle, path.c_str(), "wb") == 0) {
		fwrite(&magic, 4, 1, fHandle);

		//Loop over the map and write each checksum to the file - HACKY
		for(auto iter = maps.begin(); iter != maps.end(); ++iter) {
			memset(mapChecksum.name, 0, 33);
			strcpy_s(mapChecksum.name, 33, iter->first.c_str());
			mapChecksum.checksum = iter->second;
			fwrite(&mapChecksum, sizeof(ChecksumEntry), 1, fHandle);
		}

		fclose(fHandle);
	} else {
		OutputDebugString("Unable to write checksum cache.");
	}
}

//todo - change when migrating to C++11
bool blacklisted(const std::string& name) {
	char* blacklist[] = {"bitmaps", "sounds", "ui", "d40", "d20",
						 "c40", "c20", "c10", "b40", "b30", "a50",
						 "a30", "a10", "loc"};

	for(int i = 0; i < sizeof(blacklist) / sizeof(char*); i++) {
		if(_stricmp(name.c_str(), blacklist[i]) == 0) {
			return true;
		}
	}

	return false;
}

void mapSearch(const std::string& expression) {
	WIN32_FIND_DATA FindFileData;
	HANDLE search;

	if((search = FindFirstFile(expression.c_str(), &FindFileData)) != INVALID_HANDLE_VALUE)  {
		do {
			char *name = FindFileData.cFileName;
			if(strncmp(productType, "halor", 5) == 0) {
				std::string find(name, strrchr(name, '.')); //strip extension from the name
				if(!findMap(find) && !blacklisted(find)) {
					addMap(name);
				}
			} else {
				addMap(name);
			}
		} while(FindNextFile(search, &FindFileData) != 0);

		FindClose(search);
	}
}

void alternateLoad() {
	std::string src = getWorkingDir() + "maps\\";
	std::string expression = src + "*.map";

	strcpy_s(mapSearchPath, MAX_PATH_SIZE, src.c_str());
	mapSearch(expression);
	strcpy_s(mapSearchPath, MAX_PATH_SIZE, "maps\\");

	if(strncmp(productType, "halor", 5) == 0) {
		expression = "maps\\*.map";
		src = "maps\\";
		strcpy_s(mapSearchPath, MAX_PATH_SIZE, src.c_str());
		mapSearch(expression);
		strcpy_s(mapSearchPath, MAX_PATH_SIZE, "maps\\");
	}
}

bool findMapFile(const char* expression, const char* buffer) {
	bool found = false;
	WIN32_FIND_DATA FindFileData;
	HANDLE search = FindFirstFile(expression, &FindFileData);

	if(search != INVALID_HANDLE_VALUE) {
		do {
			if(_stricmp(FindFileData.cFileName, buffer) == 0) {
				found = true;
				break;
			}
		} while(FindNextFile(search, &FindFileData) != 0);
		
		FindClose(search);
	}

	return found;
}

//Switch to array search
void locateMapPath(const char* name) {
	char buff[128];
	strcpy_s(buff, 128, name);
	strcat_s(buff, 128, ".map");
	std::string haloExpression = "maps\\*.map";
	std::string hacPath = getWorkingDir() + "maps\\";
	std::string hacExpression = hacPath + "*.map";

	//Scan default folder first
	if(findMapFile(haloExpression.c_str(), buff)) {
		strcpy_s(mapSearchPath, 256, "maps\\");
	} else if(findMapFile(hacExpression.c_str(), buff)) {
		strcpy_s(mapSearchPath, 256, hacPath.c_str());
	} else {
		OutputDebugString("Unable to locate map");
	}
}

void _declspec(naked) SearchPathSwitchCC() {
	_asm {
		push eax
		call locateMapPath
		add esp, 4
		jmp BackToPathLoad
	}
}

void _declspec(naked) WriteCacheCC() {
	_asm {
		call alternateLoad
		movzx eax, cacheUpdate
		test eax, eax
		jz Continue
		call writeCache

		//Original
		Continue:
		jmp BackToLoading
	}
}

/*
 * Reads the checksum cache file and loads the checksums into a hashmap.
 * This function is called just before the map checksum loop begins.
 */
void loadChecksums() {
	FILE* fHandle;
	std::string path = getWorkingDir() + FILE_NAME;

	if(fopen_s(&fHandle, path.c_str(), "rb") == 0) {
		int magic, numEntries = 0;
		bool damaged = false;
		fseek(fHandle, 0L, SEEK_END);
		size_t size = ftell(fHandle);

		if(size > 4) {
			numEntries = (size - 4) / sizeof(ChecksumEntry);
		}

		std::vector<ChecksumEntry> entries(numEntries);

		//Continue loading checksums
		rewind(fHandle);
		fread_s(&magic, 4, 4, 1, fHandle);

		if(magic == CACHE_MAGIC) {
			fread_s(&entries.front(), sizeof(ChecksumEntry) * numEntries, sizeof(ChecksumEntry), numEntries, fHandle);
			
			//Add entries to map
			for(int i = 0; i < numEntries; ++i) {
				maps[entries[i].name] = entries[i].checksum;
			}
		} else {
			damaged = true; //@todo sub for CRC
		}

		fclose(fHandle);

		if(damaged) {
			_unlink(path.c_str());
		}
	}
}

/*
 * Run each time Halo loads a map during start-up.
 * The calculated checksum is pushed to the stack and then saved to the cache.
 */
void _declspec(naked) AddChecksumCC() {
	__asm {
		push ecx
		call AddChecksum
		add esp, 4
		jmp ContinueChecksum
	}
}

/*
 * Checks to see a checksum is already contained within the hash-map.
 * If a checksum is found, it is loaded from the hash-map and stored within
 * Halo's map table entry that corresponds with char* map.
 */
bool _stdcall findChecksum(const char* map, std::uint32_t& checksum) {
	if((maps.find(map)) != maps.end()) {
		checksum = maps[map];
		return true;
	} else {
		currentMap = map;
		maps[map] = 0;
	}

	return false;
}

/*
 * Checks to see whether the cache already contains the checksum for the map.
 * If the checksum exists, it will be loaded into the corresponding table entry
 * for the map and Halo's checksum algorithm will be skipped. Otherwise, we will
 * allow Halo to checksum the map and it will be added to the cache at the end.
 */
void _declspec(naked) LoadFix() {
	__asm {
		//Hook code
		pushfd
		pushad
		push eax
		push ecx
		call findChecksum
		test al, al
		popad
		jnz loadCachedChecksum
		
		//Continue checksum function
		popfd
		sub esp, 0x1C
		push ebx
		push ebp
		jmp DoChecksum
		
		//Bypass checksum function
		loadCachedChecksum:
		popfd
		ret
	}
}

/*
 * Empties the checksum cache file.
 */
bool flushCache() {
	std::string path = getWorkingDir() + FILE_NAME;
	return _unlink(path.c_str()) == 0;
}