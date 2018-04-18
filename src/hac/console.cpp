#include "console.h"
#include "EnginePointers.h"
#include <stdexcept>
#include <string>
#include <sstream>

std::uintptr_t ConsolePrint;

const float yellow[4] = {1.0f, 1.0f, 1.0f, 0.4f};
const float red[4] = {1.0f, 1.0f, 0.0f, 0.0f};
const float green[4] = {1.0f, 0.0f, 1.0f, 0.0f};
const float blue[4] = {1.0f, 0.0f, 0.4f, 1.0f};

void hkDrawText(const char* pString, const float* fColour) {
    __asm {
        mov eax, fColour
        push pString
        call ConsolePrint
        add esp, 4
    }
}

const float* colourSelect(C_TEXT_COLOUR colour) {
	const float* tColour = yellow;

	switch(colour) {
		case C_TEXT_YELLOW:
			tColour = yellow;
			break;
		case C_TEXT_RED:
			tColour = red;
			break;
		case C_TEXT_BLUE:
			tColour = blue;
			break;
		case C_TEXT_GREEN:
			tColour = green;
			break;
		default:
			throw std::runtime_error("Unhandled colour enum value!");
	}

	return tColour;
}

void hkDrawText(const char* pString, C_TEXT_COLOUR colour) {
	const float* tColour = colourSelect(colour);
	std::istringstream ifs(pString);

	std::string line;

	while(getline(ifs, line, '\r')) {
		hkDrawText(line.c_str(), tColour);
	}
}

void hkDrawText(const std::string& text, C_TEXT_COLOUR colour) {
	hkDrawText(text.c_str(), colour);
}