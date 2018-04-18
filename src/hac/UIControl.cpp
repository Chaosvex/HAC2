#include "UIControl.h"
#include "DebugHelper.h"
#include "EngineTypes.h"
#include <cstdint>

using namespace EngineTypes;

TagTableHeader* pTagTableHeader;

const std::uint32_t USTR = 0x75737472; //todo move

uint32_t tagSearch(std::uint32_t tagClass, char* tagName) {
	TagEntry* table = pTagTableHeader->TagTableEntryPointer;

	std::uint32_t entries = pTagTableHeader->TagCount;
	std::uint32_t tagID = 0;

	for(uint32_t i = 0; i < entries; i++) {
		if(table[i].class0 == tagClass) {
			if(strcmp(table[i].name, tagName) == 0) {
				tagID = table[i].tagID;
				break;
			}
		}
	}

	return tagID;
}

void* stringEntry(std::uint32_t index) {
	std::uint32_t tagID = tagSearch(USTR, "ui\\shell\\strings\\loading");

	if(tagID == NULL) {
		return NULL;
	}

	std::uint32_t tagTable = reinterpret_cast<uint32_t>(pTagTableHeader->TagTableEntryPointer);
	
	if(tagTable == NULL) {
		return NULL;
	}

	tagID &= 65535;
	tagID <<= 5;

	std::uint32_t* stringEntries = (uint32_t*)*(uint32_t*)(tagTable + tagID + 20);

	//Check for out of bounds
	if(index > *stringEntries) {
		return NULL;
	}

	std::uint32_t* pCharArr = (uint32_t*)*(uint32_t*)((uint32_t)stringEntries + 4);
	index += index * 4;
	void* stringInfo = (void*)((uint32_t)pCharArr + (index * 4));

	return stringInfo;
}