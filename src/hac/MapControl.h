#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <cstdint>

#define MAX_PATH_SIZE 256

extern DWORD* MapTableSize;

void addChecksum(const std::string& map, std::uint32_t checksum);
bool findMap(const std::string& name);
void addMap(const char* name);
void loadChecksum(std::string map, bool ceDir = false);
bool mapCheck(const char* response);
void copyMaps();