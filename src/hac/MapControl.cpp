#include "MapControl.h"
#include "mapCache.h" //remove in future
#include "EnginePointers.h"
#include "QueryParser.h"
#include "EngineTypes.h"
#include "misc.h"

using namespace EngineTypes;

DWORD* MapTableSize;
EngineTypes::MapEntry** pMapTable;
char* pCurrentMap;

/*
 * Return true if the map is found and no action is required
 */
bool mapCheck(const char* response) {
	QueryParser parser(response);
	std::string mapName = parser.getValue("mapname");
	return findMap(mapName);
}

/*
 * Because the struct for CE and PC are different sizes
 * and we use the same build for both of them, we have 
 * to calculate the pointer address at runtime.
 */
bool findMap(const std::string& name) {
	MapEntry* entries = *pMapTable;
	int entrySize = sizeof(MapEntry);
	
	if(strncmp(productType, "halom", 5) == 0) {
		entrySize = sizeof(MapEntryCE);
	}

	for(size_t i = 0; i < *MapTableSize; i++) {
		char* entryName = ((MapEntry*)(((DWORD)entries) + (entrySize * i)))->name;
		if(_stricmp(name.c_str(), entryName) == 0) {
			return true;
		}
	}
	
	return false;
}

void addChecksum(const std::string& map, std::uint32_t checksum) {
	maps[map] = checksum;
}

/*
 * If HAC has installed maps to its own folder (in cases where we have
 * no write access to Halo's folder), this function will be called
 * to move them to Halo's folder if we're ever able. This ensures the
 * maps are available to all users on the machine and that they can be
 * played if HAC is deleted.
 */
void copyMaps() {
	WIN32_FIND_DATA FindFileData;
	HANDLE search;
	char destPath[] = "maps\\";
	std::string srcPath = getWorkingDir() + destPath;
	std::string searchStr = srcPath + "*.map";

	if((search = FindFirstFile(searchStr.c_str(), &FindFileData)) !=  INVALID_HANDLE_VALUE) {
		char buff[128];
		do {
			memset(buff, 0, 128);
			std::string filePath = srcPath + FindFileData.cFileName;
			strcat_s(buff, 128, destPath);
			strcat_s(buff, 128, FindFileData.cFileName);
			MoveFile(filePath.c_str(), buff);
		} while(FindNextFile(search, &FindFileData) != 0);
	}
}