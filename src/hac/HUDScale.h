#pragma once

typedef unsigned char BYTE;

namespace HUDScale {
	void ratioFix(bool command = false);
	void reset(bool bg_fix = true);
	void adjust(BYTE* pMouseBuffer);
	void toggle();
};