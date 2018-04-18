#pragma once

#include <string>

enum C_TEXT_COLOUR {
	C_TEXT_RED, C_TEXT_GREEN, C_TEXT_BLUE, C_TEXT_YELLOW
};

void hkDrawText(const std::string& text, C_TEXT_COLOUR colour);
void hkDrawText(const char* text, C_TEXT_COLOUR colour);