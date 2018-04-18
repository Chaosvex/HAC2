#pragma once

#include <cstdint>

namespace SightJacker {
	bool isEnabled();
	void enable();
	void disable(bool surpress = false);
	void toggle();
	void switchPlayer(const unsigned char* pMouseBuffer, unsigned char* pKeyboardBuffer);
	void draw();
	void lost();
	void reset();
	void playerDied(std::uint32_t index);
};