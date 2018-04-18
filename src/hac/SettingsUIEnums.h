#pragma once

namespace SettingsUI {

//@todo - update VS as always and remove the hacks

enum /*class*/ ChatRenderer { 
	HALO, HAC2, HIDE, CR_END
};

enum /*class*/ LineLimit {
	ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, LL_END
};

enum /*class*/ ChatDisplayTime {
	_FOUR, _FIVE, _SIX, _SEVEN, _EIGHT, _NINE, _TEN, DT_END
};

enum /*class*/ ChatAlign {
	LEFT, RIGHT, TA_END
};

enum /*class*/ YesNo { //well...
	NO, YES, YN_END
};

enum /*class*/ TypefaceDefaults {
	DEFAULT, HALO_DEFAULT, TD_END
};

enum /*class*/ FontSize {
	TWENTY, TWENTY_TWO, TWENTY_FOUR, TWENTY_SIX, TWENTY_EIGHT,
	THIRTY, THIRTY_TWO, THIRTY_FOUR, THIRTY_SIX, THIRTY_EIGHT,
	FOURTY, FOURTY_TWO, FOURTY_FOUR, FOURTY_SIX, FOURTY_EIGHT,
	FIFTY, FS_END
};

enum /*class*/ MessageDisplay {
	HUD_ONLY, CHAT_ONLY, HUD_AND_CHAT, MD_END
};

enum /*class*/ ChatSettings {
	CHAT_RENDERER, LINE_LIMIT, DISPLAY_TIME, CHAT_ALIGN, COMBAT_ALIGN, CURVED_EFFECT,
	TYPEFACE, FONT_SIZE, LOG_MESSAGES, CS_END
};

enum /*class*/ GeneralSettings {
	WIDESCREEN_HUD, SCOPE_BLUR, ACTIVE_CAMO, PLAY_DURING_QUEUE, GS_END
};

enum /*class*/ OpticSettings {
	MEDAL_PACK, MEDAL_MESSAGES, OS_END
};

};