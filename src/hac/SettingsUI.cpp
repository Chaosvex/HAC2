#include "SettingsUI.h"
#include "EngineTypes.h"
#include "EnginePointers.h"
#include "Patcher.h"
#include "PatchGroup.h"
#include "Codefinder.h"
#include "DebugHelper.h"
#include "CustomChat.h"
#include "HUDScale.h"
#include "SettingsUIEnums.h"
#include "ScopeBlur.h"
#include "Preferences.h"
#include <vector>
#include <memory>

using namespace EngineTypes;

namespace SettingsUI {

struct Bounds {
	Bounds(std::uint16_t lower, std::uint16_t upper) {
		this->lower = lower;
		this->upper = upper;
	}
	std::uint16_t lower;
	std::uint16_t upper;
};

std::uint16_t DisplayTimeMapping[] = { 4, 5, 6, 7, 8, 9, 10 };
std::uint16_t LineLimitMapping[]   = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
std::uint16_t FontSizeMapping[]    = { 20, 22, 24, 26, 28, 30, 32, 34, 36,
                                       38, 40, 42, 44, 46, 48, 50 };


typedef std::vector<std::pair<std::uint16_t, Bounds>> SliderValues;
typedef UICallbackAction (*UI_EVENT_HANDLER)(UIWidgetGroup*, void*, void*);
typedef int (*GAME_DATA_INPUT_HANDLER)(UIWidgetGroup*);

std::unique_ptr<PatchGroup> patches = nullptr;
UI_EVENT_HANDLER eventHandlers[UI_EVENT_CALLBACKS_END];
GAME_DATA_INPUT_HANDLER gameDataHandlers[DATA_INPUT_FUNCTIONS_END];
SliderValues chatPrefs, opticPrefs, controlPrefs, generalPrefs;

SliderValues generalGetDefaults();
SliderValues opticGetDefaults();
SliderValues controlsGetDefaults();
SliderValues chatGetDefaults();
void updateIntermediatePreferences(SliderValues& prefs, const std::vector<std::uint16_t>& values);
int __cdecl opticMenuUpdate(UIWidgetGroup* group);
int __cdecl chatMenuUpdate(UIWidgetGroup* group);
UICallbackAction __cdecl generalSetDefaults(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl generalInitialiseOptions(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl generalSave(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl chatSetDefaults(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl chatInitialiseOptions(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl chatDisposeOptions(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl chatSave(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl controlsSetDefaults(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl controlsInitialiseOptions(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl controlsSave(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl opticSetDefaults(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl opticInitialiseOptions(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl opticDisposeOptions(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl opticSave(UIWidgetGroup* group, void*, void*);
UICallbackAction hacOptionsBeginEditing(UIWidgetGroup* group, void*, void*);
UICallbackAction hacOptionsEndEditing(UIWidgetGroup* group, void*, void*);
UICallbackAction hacOptionsSet(UIWidgetGroup* group, void*, void*);
UICallbackAction __cdecl hacOptionsDispose(UIWidgetGroup* group, void*, void*);
void setChatSpinners(UIWidgetGroup* group);
void setSpinners(UIWidgetGroup* widget, const SliderValues& values);
std::vector<std::uint16_t> getSpinners(UIWidgetGroup* group, int spinnerCount);
bool playerEditingProfile = false;
bool stalePreferences = false;

void appendEventHandlers() {
	short signature[] = {0xFF, 0x14, 0x8D, -1, -1, -1, -1, 0x83, 0xC4, 0x0C, 0x84, 0xC0};
	std::uintptr_t address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address == NULL) {
		throw HookException("appendEventHandlers failed.");
	}
	
	UI_EVENT_HANDLER* original = *reinterpret_cast<UI_EVENT_HANDLER**>(address += 3);
	memcpy_s(eventHandlers, UI_EVENT_CALLBACKS_END * 4, original, (LOAD_GAME_MENU_DELETE_FINISH + 1) * 4);
	
	eventHandlers[GENERAL_SCREEN_INIT] = generalInitialiseOptions;
	eventHandlers[GENERAL_SCREEN_DEFAULTS] = generalSetDefaults;
	eventHandlers[GENERAL_SCREEN_SET] = generalSave;

	eventHandlers[OPTIC_SCREEN_INIT] = opticInitialiseOptions;
	//eventHandlers[OPTIC_SCREEN_DISPOSE] = opticDisposeOptions;
	eventHandlers[OPTIC_SCREEN_DEFAULTS] = opticSetDefaults;
	eventHandlers[OPTIC_SCREEN_SET] = opticSave;

	eventHandlers[CHAT_SCREEN_INIT] = chatInitialiseOptions;
	//eventHandlers[CHAT_SCREEN_DISPOSE] = chatDisposeOptions
	eventHandlers[CHAT_SCREEN_DEFAULTS] = chatSetDefaults;
	eventHandlers[CHAT_SCREEN_SET] = chatSave;

	eventHandlers[HAC_CONTROLS_SCREEN_INIT] = controlsInitialiseOptions;
	eventHandlers[HAC_CONTROLS_SCREEN_DEFAULTS] = controlsSetDefaults;
	eventHandlers[HAC_CONTROLS_SET] = controlsSave;

	eventHandlers[HAC_OPTIONS_BEGIN_EDITING] = hacOptionsBeginEditing;
	eventHandlers[HAC_OPTIONS_END_EDITING] = hacOptionsEndEditing;
	eventHandlers[HAC_OPTIONS_SAVE_PREFERENCES] = hacOptionsSet;
}

void appendDataInputHandlers() {
	short signature[] = {0xFF, 0x14, 0x85, -1, -1, -1, -1, 0x83, 0xC4, 0x04, 0x8B};
	std::uintptr_t address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address == NULL) {
		throw HookException("appendDataInputHandlers failed.");
	}
	
	GAME_DATA_INPUT_HANDLER* original = *reinterpret_cast<GAME_DATA_INPUT_HANDLER**>(address += 3);
	memcpy_s(gameDataHandlers, DATA_INPUT_FUNCTIONS_END * 4, original, (NETWORK_SETTINGS_UPDATE + 1) * 4);
	
	gameDataHandlers[OPTIC_MENU_UPDATE] = opticMenuUpdate;
	gameDataHandlers[CHAT_MENU_UPDATE] = chatMenuUpdate;
}

void install() {
	appendEventHandlers();
	appendDataInputHandlers();

	patches = std::unique_ptr<PatchGroup>(new PatchGroup());
	
	{ //Get Halo to use our event handler callbacks
		std::uint8_t replacement[sizeof(void*)];
		UI_EVENT_HANDLER* pHandlers = eventHandlers;
		memcpy(replacement, &pHandlers, sizeof(void*));
		short signature[] = {0xFF, 0x14, 0x8D, -1, -1, -1, -1, 0x83, 0xC4, 0x0C};
		CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
		for(auto i = cf.begin(); i != cf.end(); i++) {
			patches->add(new PatchHook(*i, 3, replacement, sizeof(replacement)));
		}
	}

	{ //Adjust event handler callback bounds checking
		short signature[] = {0x66, 0x3D, 0xBE, 0x00, 0x73};
		std::uint16_t replacement = UI_EVENT_CALLBACKS_END;
		CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
		for(auto i = cf.begin(); i != cf.end(); i++) {
			patches->add(new PatchHook(*i, 2, reinterpret_cast<std::uint8_t*>(&replacement), sizeof(replacement)));
		}
	}

	{ //Get Halo to use our game data input callbacks
		std::uint8_t replacement[sizeof(void*)];
		GAME_DATA_INPUT_HANDLER* pHandlers = gameDataHandlers;
		memcpy(replacement, &pHandlers, sizeof(void*));
		short signature[] = {0xFF, 0x14, 0x85, -1, -1, -1, -1, 0x83, 0xC4, 0x04, 0x8B};
		CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
		for(auto i = cf.begin(); i != cf.end(); i++) {
			patches->add(new PatchHook(*i, 3, replacement, sizeof(replacement)));
		}
	}

	{ //Adjust game data callback bounds checking
		short signature[] = {0x66, 0x3D, 0x3B};
		std::uint16_t replacement = DATA_INPUT_FUNCTIONS_END;
		CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
		for(auto i = cf.begin(); i != cf.end(); i++) {
			patches->add(new PatchHook(*i, 2, reinterpret_cast<std::uint8_t*>(&replacement), sizeof(replacement)));
		}
	}

	if(!patches->install()) {
		patches.reset();
		throw HookException("Settings UI hooks failed.");
	}
}

void uninstall() {
	patches.reset();
}

void tempPreferencesDispose() {
	generalPrefs.clear();
	chatPrefs.clear();
	opticPrefs.clear();
	controlPrefs.clear();
}

int find_index(std::uint16_t* array, std::size_t len, std::uint16_t value) {
	return std::distance(array, std::find(array, array + len, value));
}

void savePreferences() {
	Preferences::add("hud_ratio_fix", generalPrefs[GeneralSettings::WIDESCREEN_HUD].first);
	Preferences::add("no_scope_blur", generalPrefs[GeneralSettings::SCOPE_BLUR].first);
	Preferences::add("active_camo_fix", generalPrefs[GeneralSettings::ACTIVE_CAMO].first);
	Preferences::add("play_during_queue", generalPrefs[GeneralSettings::PLAY_DURING_QUEUE].first);
	Preferences::add("optic_pack", "er");
	Preferences::add("optic_redirect", opticPrefs[OpticSettings::MEDAL_MESSAGES].first);
	Preferences::add("custom_chat", chatPrefs[ChatSettings::CHAT_RENDERER].first);
	Preferences::add("line_limit",  LineLimitMapping[chatPrefs[ChatSettings::LINE_LIMIT].first]);
	Preferences::add("display_time", DisplayTimeMapping[chatPrefs[ChatSettings::DISPLAY_TIME].first]);
	Preferences::add("chat_align", chatPrefs[ChatSettings::CHAT_ALIGN].first);
	Preferences::add("combat_align", chatPrefs[ChatSettings::COMBAT_ALIGN].first);
	Preferences::add("curved_effect", chatPrefs[ChatSettings::CURVED_EFFECT].first);
	Preferences::add("typeface", chatPrefs[ChatSettings::TYPEFACE].first);
	Preferences::add("font_size", FontSizeMapping[chatPrefs[ChatSettings::FONT_SIZE].first]);
	Preferences::add("log_messages", chatPrefs[ChatSettings::LOG_MESSAGES].first);
}

int __cdecl chatMenuUpdate(UIWidgetGroup* group) {
	DebugHelper::Translate("CM called");
	return 0;
}

int __cdecl opticMenuUpdate(UIWidgetGroup* group) {
	DebugHelper::Translate("OC called");
	return 0;
}

void reinitialiseSystems() {
	std::uint16_t chatRenderer = Preferences::find("custom_chat", 0);

	switch(chatRenderer) {
		case SettingsUI::ChatRenderer::HAC2:
			Chat::enable();
			break;
		case SettingsUI::ChatRenderer::HIDE:
			Chat::enable(true);
			break;
		case SettingsUI::ChatRenderer::HALO:
			Chat::disable();
			break;
	}

	HUDScale::reset();

	if(Preferences::find("hac_widescreen", true)) {
		HUDScale::ratioFix(true);
	}

	if(Preferences::find("active_camo_fix", true)) {
		*activeCamoDisable = false;
	} else {
		*activeCamoDisable = true;
	}

	if(Preferences::find("no_scope_blur", true)) {
		ScopeBlurFix::disable();
	} else {
		ScopeBlurFix::enable();
	}
}

UICallbackAction hacOptionsSet(UIWidgetGroup* group, void*, void*) {
	if(stalePreferences) {
		savePreferences();
		reinitialiseSystems();
	}

	tempPreferencesDispose();
	playerEditingProfile = false;
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction hacOptionsEndEditing(UIWidgetGroup* group, void*, void*) {
	tempPreferencesDispose();
	playerEditingProfile = false;
	return UICallbackAction::CONTINUE_ACTION;
}

// welcome to converting between Halo's preferences system and HAC's
UICallbackAction hacOptionsBeginEditing(UIWidgetGroup* group, void*, void*) {
	if(playerEditingProfile) {
		return UICallbackAction::CONTINUE_ACTION;
	}

	SliderValues general = generalGetDefaults();
	std::pair<std::uint16_t, Bounds>& default = general[WIDESCREEN_HUD];
	generalPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("hud_ratio_fix", default.first), default.second));
	default = general[SCOPE_BLUR];
	generalPrefs.emplace_back(std::make_pair(!Preferences::find<std::uint16_t>("no_scope_blur", default.first), default.second));
	default = general[ACTIVE_CAMO];
	generalPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("active_camo_fix", default.first), default.second));
	default = general[PLAY_DURING_QUEUE];
	generalPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("play_during_queue", default.first), default.second));

	SliderValues optic = opticGetDefaults();
	opticPrefs.emplace_back(std::make_pair(0, Bounds(0, 1))); //generated by a function later on
	default = optic[MEDAL_MESSAGES];
	opticPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("optic_redirect", default.first), default.second));

	SliderValues chatDefs = chatGetDefaults();
	default = chatDefs[CHAT_RENDERER];
	chatPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("custom_chat", default.first), default.second));

	// chat line limit
	default = chatDefs[LINE_LIMIT];
	std::uint16_t line_limit = Preferences::find<std::uint16_t>("line_limit", default.first);
	std::uint16_t line_limit_index = find_index(LineLimitMapping, sizeof(LineLimitMapping), line_limit);
	chatPrefs.emplace_back(std::make_pair(line_limit_index, default.second));

	// chat display time
	default = chatDefs[DISPLAY_TIME];
	std::uint16_t display_time = Preferences::find<std::uint16_t>("display_time", default.first);
	std::uint16_t display_time_index = find_index(DisplayTimeMapping, sizeof(DisplayTimeMapping), display_time);
	chatPrefs.emplace_back(std::make_pair(display_time_index, default.second));

	default = chatDefs[CHAT_ALIGN];
	chatPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("chat_align", default.first), default.second));
	default = chatDefs[COMBAT_ALIGN];
	chatPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("combat_align", default.first), default.second));
	default = chatDefs[CURVED_EFFECT];
	chatPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("curved_effect", default.first), default.second));
	default = chatDefs[TYPEFACE];
	chatPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("typeface", default.first), default.second));

	// chat font size
	default = chatDefs[FONT_SIZE];	
	std::uint16_t font_size = Preferences::find<std::uint16_t>("font_size", default.first);
	std::uint16_t font_size_index = find_index(FontSizeMapping, sizeof(FontSizeMapping), font_size);
	chatPrefs.emplace_back(std::make_pair(font_size_index, default.second));

	default = chatDefs[LOG_MESSAGES];
	chatPrefs.emplace_back(std::make_pair(Preferences::find<std::uint16_t>("log_messages", default.first), default.second));

	playerEditingProfile = true;
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction generalSave(UIWidgetGroup* group, void*, void*) {
	std::vector<std::uint16_t> values = getSpinners(group->parent->parent, GeneralSettings::GS_END);
	updateIntermediatePreferences(generalPrefs, values);
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction opticSave(UIWidgetGroup* group, void*, void*) {
	std::vector<std::uint16_t> values = getSpinners(group->parent->parent, OpticSettings::OS_END);
	updateIntermediatePreferences(opticPrefs, values);
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction chatSave(UIWidgetGroup* group, void*, void*) {
	std::vector<std::uint16_t> values = getSpinners(group->parent->parent, ChatSettings::CS_END);
	updateIntermediatePreferences(chatPrefs, values);
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction controlsSave(UIWidgetGroup* group, void*, void*) {
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction generalSetDefaults(UIWidgetGroup* group, void*, void*) {
	setSpinners(group->parent->parent, generalGetDefaults());
	return UICallbackAction::CONTINUE_ACTION;
}

void updateIntermediatePreferences(SliderValues& prefs, const std::vector<std::uint16_t>& values) {
	if(values.size() != prefs.size()) {
		hkDrawText("updateIntermediatePreferences is borked", C_TEXT_RED);
		return;
	}

	for(int i = 0, j = values.size(); i < j; i++) {
		Bounds bounds = prefs[i].second;
		std::uint16_t index = values[i];
		if(index >= bounds.lower && index < bounds.upper) {
			if(index != prefs[i].first) {
				prefs[i].first = index;
				stalePreferences = true;
			}
		}
	}
}

SliderValues generalGetDefaults() {
	SliderValues values;
	values.emplace_back(std::make_pair(YesNo::YES, Bounds(0, YesNo::YN_END)));
	values.emplace_back(std::make_pair(YesNo::YES, Bounds(0, YesNo::YN_END)));
	values.emplace_back(std::make_pair(YesNo::YES, Bounds(0, YesNo::YN_END)));	
	values.emplace_back(std::make_pair(YesNo::NO, Bounds(0, YesNo::YN_END)));
	return values;
}

SliderValues controlsGetDefaults() {
}

SliderValues opticGetDefaults() {
	SliderValues values;
	values.emplace_back(std::make_pair(YesNo::NO, Bounds(0, YesNo::YN_END)));
	values.emplace_back(std::make_pair(MessageDisplay::HUD_ONLY, Bounds(0, MessageDisplay::MD_END)));
	return values;
}

SliderValues chatGetDefaults() {
	SliderValues values;
	values.emplace_back(std::make_pair(ChatRenderer::HAC2, Bounds(0, ChatRenderer::CR_END)));
	values.emplace_back(std::make_pair(LineLimit::SIX, Bounds(0, LineLimit::LL_END)));
	values.emplace_back(std::make_pair(ChatDisplayTime::_SEVEN, Bounds(0, ChatDisplayTime::DT_END)));	
	values.emplace_back(std::make_pair(ChatAlign::LEFT, Bounds(0, ChatAlign::TA_END)));
	values.emplace_back(std::make_pair(ChatAlign::LEFT, Bounds(0, ChatAlign::TA_END)));
	values.emplace_back(std::make_pair(YesNo::YES, Bounds(0, YesNo::YN_END)));
	values.emplace_back(std::make_pair(TypefaceDefaults::DEFAULT, Bounds(0, TypefaceDefaults::TD_END)));
	values.emplace_back(std::make_pair(FontSize::THIRTY_FOUR, Bounds(0, FontSize::FS_END)));
	values.emplace_back(std::make_pair(YesNo::NO, Bounds(0, YesNo::YN_END)));
	return values;
}

UICallbackAction chatSetDefaults(UIWidgetGroup* group, void*, void*) {
	setSpinners(group->parent->parent, chatGetDefaults());
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction opticSetDefaults(UIWidgetGroup* group, void*, void*) {
	setSpinners(group->parent->parent, opticGetDefaults());
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction controlsSetDefaults(UIWidgetGroup* group, void*, void*) {
	SliderValues values;
	setSpinners(group->parent->parent, values);
	return UICallbackAction::CONTINUE_ACTION;
}

/* Initialisation functions, called when the matching screen is entered */
UICallbackAction generalInitialiseOptions(UIWidgetGroup* group, void*, void*) {
	setSpinners(group, generalPrefs);
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction controlsInitialiseOptions(UIWidgetGroup* group, void*, void*) {
	setSpinners(group, controlPrefs);
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction opticInitialiseOptions(UIWidgetGroup* group, void*, void*) {
	setSpinners(group, opticPrefs);
	return UICallbackAction::CONTINUE_ACTION;
}

UICallbackAction chatInitialiseOptions(UIWidgetGroup* group, void*, void*) {
	setSpinners(group, chatPrefs);
	return UICallbackAction::CONTINUE_ACTION;
}
/* End of initialisation functions */

std::vector<std::uint16_t> getSpinners(UIWidgetGroup* group, int spinnerCount) {
	std::vector<std::uint16_t> values;
	UIWidgetGroup* listItem = group->child;

	for(int i = 0; i < spinnerCount; i++) {
		UIWidgetGroup* element = listItem->child;
		
		while(element->index != 2) {
			element = element->next;

			if(!element->index) {
				//error
			}
		}
		
		values.emplace_back(element->stringIndex);
		listItem = listItem->next;
	}

	return values;
}

void setSpinners(UIWidgetGroup* group, const SliderValues& values) {
	UIWidgetGroup* listItem = group->child; //column list widget to first child container

	for(auto i = values.begin(); i != values.end(); i++) {
		UIWidgetGroup* element = listItem->child; //child container to first element (label or spinner)
		
		while(element->index != 2) {
			element = element->next;

			if(!element->index) {
				//error
			}
		}

		if(i->first < i->second.lower || i->first >= i->second.upper) {
			element->stringIndex = i->second.lower;
		} else {
			element->stringIndex = i->first;
		}

		listItem = listItem->next;
	}
}

template<typename T>
std::vector<T*> tagSearch(uint32_t tagClass, const char* tagName = NULL) {
	TagEntry* table = pTagTableHeader->TagTableEntryPointer;
	std::vector<T*> tags;

	for(uint32_t i = 0; i < pTagTableHeader->TagCount; i++) {
		if(table[i].class0 == tagClass) {
			if(tagName == NULL || _stricmp(table[i].name, tagName) == 0) {
				tags.emplace_back(reinterpret_cast<T*>(table[i].tagStruct));
			}
		}
	}

	return tags;
}

void testFunc() {
	std::vector<UIWidget*> widgets = tagSearch<UIWidget>(DELA, "ui\\shell\\main_menu\\settings_select\\player_setup\\player_profile_edit\\video_settings\\video_options_menu");
	
	if(widgets.empty()) {
		DebugHelper::DisplayAddress(1337, 10);
	}

	UIWidget* widget = widgets[0];
	DebugHelper::DisplayAddress((DWORD)&(widget->functions.tag[0]), 16);
	//DebugHelper((DWORD)&widget->eventHandlers.tag[1].function, 16);
}

};