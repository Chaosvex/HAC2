#pragma once

typedef unsigned char BYTE;

namespace HUDColour {
	struct RGBColour {
		double red, green, blue;
	};

	struct HSVColour {
		double hue, saturation, value;
	};

	enum ToggleState {
		RETICLE_COLOUR,
		PRIMARY_COLOUR,
		SECONDARY_COLOUR,
		TERTIARY_COLOUR,
		DISABLED
	};

	void adjust(BYTE* pMouseBuffer);
	void toggle();
	void colour(const HSVColour& offsets, ToggleState state);
	void reset();
	void clear();
	void loadState();

	HSVColour rgb2hsv(RGBColour& rgb);
};