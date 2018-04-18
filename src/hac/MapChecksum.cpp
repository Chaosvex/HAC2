#include "MapChecksum.h"
#include "EngineTypes.h"

using namespace std;
using namespace EngineTypes;

MapChecksum::MapChecksum(const string& path) : filepath(path) {
	map = INVALID_HANDLE_VALUE;
	file = INVALID_HANDLE_VALUE;
	initXorBuff();
	createFileMapping();
}

MapChecksum::~MapChecksum() {
	UnmapViewOfFile(view);
	CloseHandle(map);
	CloseHandle(file);
}

void MapChecksum::xorChunk(std::uint32_t& checksum, char* data, std::uint32_t length, std::uint32_t* xor) {
	for(std::uint32_t i = 0; i < length; ++i) {
		int byte = data[i];
		byte ^= checksum;
		byte &= 255;
		byte = xor[byte];
		checksum /= 256;
		checksum ^= byte;
	}
}

void MapChecksum::initXorBuff() {
	for(DWORD i = 0, val = 0; i < 256; i++, val = i) {
		for(int j = 0; j < 8; j++) {
			if(val % 2 == 1) {
				val /= 2;
				val ^= 0xEDB88320;
			} else {
				val /= 2;
			}
		}
		this->xorBuffer[i] = val;
	}
}

void MapChecksum::createFileMapping() {
	file = CreateFile(filepath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 

	if(file == INVALID_HANDLE_VALUE) {
		throw ChecksumException("Unable to open map file for checksumming.");
	}

	map = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);

	if(map == NULL) {
		throw ChecksumException("Unable to create mapping.");
	}

	view = static_cast<BYTE*>(MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0));

	if(view == NULL) {
		throw ChecksumException("Unable to create view.");
	}
}

DWORD MapChecksum::checksum() {
	std::uint32_t checksum = -1;
	DWORD offset = (DWORD)view;
	LARGE_INTEGER fileSize = {};

	if(GetFileSizeEx(file, &fileSize) == 0) {
		throw ChecksumException("Failed to retrieve file size.");
	}

	//Read the index
	MapHeader* pHeader = (MapHeader*)view;
	TagTableHeader *pIndex = (TagTableHeader*)(pHeader->IndexOffset + offset);

	//Ensure offsets aren't larger than the map file - cannot trust header info
	if(pHeader->IndexOffset > fileSize.QuadPart || pHeader->MetaSize > fileSize.QuadPart) {
		throw ChecksumException("Map file appears to be damaged.");
	}

	//Calculate 'rebase' value
	DWORD magic = ((DWORD)pIndex->TagTableEntryPointer - pHeader->IndexOffset) - 40;

	//Checksum SBSPs - bless this mess
	TagEntry *pTag = (TagEntry*)(((std::uint32_t)(pIndex->TagTableEntryPointer) - magic) + offset);
	SceneryTag* pScenery = (SceneryTag*)(((pTag->tagStruct - magic) + 0x5A4) + offset);
	
	for(std::uint32_t i = 0; i < pScenery->sections; ++i) {
		BSPChunkMeta* pBspMeta = (BSPChunkMeta*)(((pScenery->address - magic) + (i * 32)) + offset);
		xorChunk(checksum, (char*)(pBspMeta->offset  + offset), pBspMeta->size, xorBuffer);
	}

	//Checksum model data
	xorChunk(checksum, (char*)(pIndex->VerticesOffset + offset), pIndex->ModelDataSize, xorBuffer);

	//Checksum tag index
	xorChunk(checksum, (char*)(pHeader->IndexOffset + offset), pHeader->MetaSize, xorBuffer);

	return checksum;
}