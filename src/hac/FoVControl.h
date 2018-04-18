#pragma once

typedef unsigned char       BYTE;

namespace FoV {
	void redirect();
	void toggle();
	void adjust(BYTE *pMouseBuffer);
	void set(float fov);
	float get();
	void reset();
}