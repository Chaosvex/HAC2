#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct DisplayString {
	std::uint32_t length;
	char unknown[8];
	wchar_t* message;
};
#pragma pack(pop)

void showOverlay(bool display);
void* stringEntry(std::uint32_t index);
