#pragma once

#include <string>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>

class ChecksumException : public std::runtime_error {
public:
	ChecksumException() : std::runtime_error("Map checksumming failed!") { }
	ChecksumException(std::string msg) : std::runtime_error(msg) { };
};

class MapChecksum {
	std::string filepath;
	std::uint32_t xorBuffer[256];
	HANDLE file, map;
	BYTE* view;
	void initXorBuff();
	void createFileMapping();
	void xorChunk(std::uint32_t& checksum, char* data, std::uint32_t length, std::uint32_t* xor);

public:
	MapChecksum(const std::string& name);
	~MapChecksum();
	DWORD checksum();
};