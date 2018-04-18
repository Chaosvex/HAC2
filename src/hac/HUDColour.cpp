#include "HUDColour.h"
#include "EnginePointers.h"
#include "EngineTypes.h"
#include "Preferences.h"
#include "EngineFunctions.h"
#include "Language.h"
#include "KeyMappings.h"
#include "DebugHelper.h"
#include "EventDispatcher.h"
#include "Shared.h"
#include <cstdint>
#include <vector>
#include <map>

using namespace keymappings;
using namespace EngineTypes;

namespace HUDColour {

RGBColour hsv2rgb(HSVColour& hsv);
HSVColour rgb2hsv(RGBColour& rgb);
void groupElements();
void saveState();
void loadState();

ToggleState state = DISABLED;

enum AdjustMode {
	BEGIN_ADJUST,
	HUE_ADJUST,
	SATURATION_ADJUST,
	VALUE_ADJUST,
} adjust_state = BEGIN_ADJUST;

HSVColour offsets = {0};
std::map<ColourInformation*, ColourInformation> original; 
std::map<ToggleState, std::vector<ColourInformation*>> groups;


double clamp(const double& component, const double& offset, double lower = 0.0);

void reset() {
	memset(&offsets, 0, sizeof(HSVColour));

	for(auto i = original.begin(); i != original.end(); i++) {
		*(*i).first = (*i).second;
	}

	HUDMessage(Language::GetString(HUD_COLOUR_RESET).c_str());
	state = RETICLE_COLOUR;
	saveState();
	state = PRIMARY_COLOUR;
	saveState();
	state = SECONDARY_COLOUR;
	saveState();
	state = TERTIARY_COLOUR;
	saveState();
}

/*
 * Clear needs to be called on each map load to prevent corruption
 * caused by attempting to reset the HUD to defaults belonging to 
 * the previous map
 */
void clear() {
	original.clear();
	groups.clear();
	groupElements();
}

void adjust(BYTE *pMouseBuffer) {
	if(state != DISABLED) {
		double offset = 0.05;

		if(pMouseBuffer[MOUSE_UP]) {
			pMouseBuffer[MOUSE_UP] = 0;
		} else if(pMouseBuffer[MOUSE_DOWN]) {
			pMouseBuffer[MOUSE_DOWN] = 0;
			offset = -offset;
		} else {
			return;
		}
		
		int message = 0;

		switch(adjust_state) {
			case HUE_ADJUST:
				offsets.hue += (offset * 200.0);
				message = HUD_HUE;
				break;
			case SATURATION_ADJUST:
				offsets.saturation = clamp(offsets.saturation, offset, -1.0);
				message = HUD_SATURATION;
				break;
			case VALUE_ADJUST:
				offsets.value = clamp(offsets.value, offset, -1.0);
				message = HUD_VALUE;
				break;
		}
		
		colour(offsets, state);
	}
}

bool willAdvance() {
	return (adjust_state == BEGIN_ADJUST || adjust_state == VALUE_ADJUST);
}

bool advanceState() {
	switch(adjust_state) {
		case BEGIN_ADJUST:
			adjust_state = HUE_ADJUST;
			HUDMessage(Language::GetWideString(HUD_HUE));
			return true;
		case HUE_ADJUST:
			adjust_state = SATURATION_ADJUST;
			HUDMessage(Language::GetWideString(HUD_SATURATION));
			break;
		case SATURATION_ADJUST:
			adjust_state = VALUE_ADJUST;
			HUDMessage(Language::GetWideString(HUD_VALUE));
			break;
		case VALUE_ADJUST:
			if(state == TERTIARY_COLOUR) {
				adjust_state = BEGIN_ADJUST;
			} else {
				adjust_state = HUE_ADJUST;
				HUDMessage(Language::GetWideString(HUD_HUE));
			}
			return true;
		default:
			HUDMessage("Something broke");
	}

	return false;
}

void loadState() {
	clear();
	HSVColour offsets = {0};
	state = RETICLE_COLOUR;
	offsets.hue = Preferences::find<double>("reticle_hue", 0.0);
	offsets.saturation = Preferences::find<double>("reticle_sat", 0.0);
	offsets.value = Preferences::find<double>("reticle_val", 0.0);
	colour(offsets, state);
	state = PRIMARY_COLOUR;
	offsets.hue = Preferences::find<double>("primary_hue", 0.0);
	offsets.saturation = Preferences::find<double>("primary_sat", 0.0);
	offsets.value = Preferences::find<double>("primary_val", 0.0);
	colour(offsets, state);
	state = SECONDARY_COLOUR;
	offsets.hue = Preferences::find<double>("secondary_hue", 0.0);
	offsets.saturation = Preferences::find<double>("secondary_sat", 0.0);
	offsets.value = Preferences::find<double>("secondary_val", 0.0);
	colour(offsets, state);
	state = TERTIARY_COLOUR;
	offsets.hue = Preferences::find<double>("tertiary_hue", 0.0);
	offsets.saturation = Preferences::find<double>("tertiary_sat", 0.0);
	offsets.value = Preferences::find<double>("tertiary_val", 0.0);
	colour(offsets, state);
	state = DISABLED;
}

void saveState() {
	switch(state) {
		case RETICLE_COLOUR:
			Preferences::add("reticle_hue", offsets.hue);
			Preferences::add("reticle_sat", offsets.saturation);
			Preferences::add("reticle_val", offsets.value);
			break;
		case PRIMARY_COLOUR:
			Preferences::add("primary_hue", offsets.hue);
			Preferences::add("primary_sat", offsets.saturation);
			Preferences::add("primary_val", offsets.value);
			break;
		case SECONDARY_COLOUR:
			Preferences::add("secondary_hue", offsets.hue);
			Preferences::add("secondary_sat", offsets.saturation);
			Preferences::add("secondary_val", offsets.value);
			break;
		case TERTIARY_COLOUR:
			Preferences::add("tertiary_hue", offsets.hue);
			Preferences::add("tertiary_sat", offsets.saturation);
			Preferences::add("tertiary_val", offsets.value);
			break;
	}	
}

void toggle() {
	if(willAdvance()) {
		saveState();
		memset(reinterpret_cast<void*>(&offsets), 0, sizeof(HSVColour));
	}

	switch(state) {
		case DISABLED:
			if(advanceState()) {
				HUDMessage(Language::GetWideString(HUD_COLOUR_RETICLE).c_str());
				PlayMPSound(_multiplayer_sound_countdown_timer_end);
				state = RETICLE_COLOUR;
			}
			break;
		case RETICLE_COLOUR:
			if(advanceState()) {
				HUDMessage(Language::GetWideString(HUD_COLOUR_PRIMARY).c_str());
				PlayMPSound(_multiplayer_sound_countdown_timer_end);
				state = PRIMARY_COLOUR;
			}
			break;
		case PRIMARY_COLOUR:
			if(advanceState()) {
				HUDMessage(Language::GetWideString(HUD_COLOUR_SECONDARY).c_str());
				PlayMPSound(_multiplayer_sound_countdown_timer_end);
				state = SECONDARY_COLOUR;
			}
			break;
		case SECONDARY_COLOUR:
			if(advanceState()) {
				HUDMessage(Language::GetWideString(HUD_COLOUR_TERTIARY).c_str());
				PlayMPSound(_multiplayer_sound_countdown_timer_end);
				state = TERTIARY_COLOUR;
			}
			break;
		case TERTIARY_COLOUR:
			if(advanceState()) {
				HUDMessage(Language::GetWideString(HUD_COLOUR_SET).c_str());
				PlayMPSound(_multiplayer_sound_countdown_timer_end);
				state = DISABLED;
				dispatcher->enqueue(std::make_shared<Event>(HUD_RECOLOURED));
			}
			break;
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

double clamp(const double& component, const double& offset, double lower) {
	double clamped = component + offset;
	
	if(clamped > 1.0) {
		clamped = 1.0;
	} else if(clamped < lower) {
		clamped = lower;
	}

	return clamped;
}

void recolour(ColourInformation& elem, const HSVColour& offsets) {
	if(original.find(&elem) == original.end()) {
		original[&elem] = elem;
	}

	ColourInformation restored = original[&elem];

	RGBColour normalised;
	normalised.red = restored.red / 255.0;
	normalised.blue = restored.blue / 255.0;
	normalised.green = restored.green / 255.0;

	HSVColour hsv = rgb2hsv(normalised);
	hsv.hue += offsets.hue;

	if(hsv.hue < 0.0) {
		hsv.hue = -hsv.hue;
	}

	hsv.hue = fmod(hsv.hue, 360.0);
	hsv.saturation = clamp(hsv.saturation, offsets.saturation);
	hsv.value = clamp(hsv.value, offsets.value);
	RGBColour rgboffset = hsv2rgb(hsv);

	elem.red = static_cast<uint8_t>(rgboffset.red * 255.0);
	elem.blue = static_cast<uint8_t>(rgboffset.blue * 255.0);
	elem.green = static_cast<uint8_t>(rgboffset.green * 255.0);
	elem.alpha = 0;
}

void groupElements() {
	std::vector<UnitHUD*> hud = tagSearch<UnitHUD>(UNHI);
	std::vector<WeaponHUD*> weapons = tagSearch<WeaponHUD>(WPHI);
	std::vector<GrenadeHUD*> ghud = tagSearch<GrenadeHUD>(GRHI);
	std::vector<ColourInformation*> primary, secondary, reticle, tertiary;

	for(auto i = hud.begin(); i != hud.end(); i++) {
		primary.emplace_back(&(*i)->health_panel_bg_default_colour);
		secondary.emplace_back(&(*i)->health_panel_bg_disabled_colour);
		tertiary.emplace_back(&(*i)->health_panel_bg_flashing_colour);
		primary.emplace_back(&(*i)->health_panel_empty_meter_colour);
		primary.emplace_back(&(*i)->health_panel_flash_meter_colour);
		primary.emplace_back(&(*i)->health_panel_max_meter_colour);
		secondary.emplace_back(&(*i)->health_panel_meter_disabled_colour);
		primary.emplace_back(&(*i)->health_panel_meter_medium_health_left_colour);
		primary.emplace_back(&(*i)->health_panel_meter_min_colour);
		primary.emplace_back(&(*i)->hud_bg_default_colour);
		tertiary.emplace_back(&(*i)->hud_bg_flashing_colour);
		secondary.emplace_back(&(*i)->hug_bg_disabled_colour);
		primary.emplace_back(&(*i)->motion_sensor_bg_default_colour);
		secondary.emplace_back(&(*i)->motion_sensor_bg_disabled_colour);
		tertiary.emplace_back(&(*i)->motion_sensor_bg_flashing_colour);
		primary.emplace_back(&(*i)->motion_sensor_foreground_default_colour);
		secondary.emplace_back(&(*i)->motion_sensor_foreground_disabled_colour);
		tertiary.emplace_back(&(*i)->motion_sensor_foreground_flashing_colour);
		primary.emplace_back(&(*i)->shield_panel_bg_default_colour);
		secondary.emplace_back(&(*i)->shield_panel_bg_disabled_colour);
		tertiary.emplace_back(&(*i)->shield_panel_bg_flashing_colour);
		secondary.emplace_back(&(*i)->shield_panel_meter_disabled_colour);
		primary.emplace_back(&(*i)->shield_panel_meter_empty_colour);
		tertiary.emplace_back(&(*i)->shield_panel_meter_flash_colour);
		primary.emplace_back(&(*i)->shield_panel_meter_max_colour);
		primary.emplace_back(&(*i)->shield_panel_meter_min_colour);
		primary.emplace_back(&(*i)->shield_panel_meter_overcharge_empty_colour);
		primary.emplace_back(&(*i)->shield_panel_meter_overcharge_min_colour);
		primary.emplace_back(&(*i)->shield_panel_meter_overcharge_max_colour);
		tertiary.emplace_back(&(*i)->shield_panel_meter_overcharge_flash_colour);

		Reflexive<AuxiliaryHUDMeter>& aux_hud = (*i)->auxiliary_hud_meters;

		for(unsigned int j = 0; j < aux_hud.count; j++) {
			primary.emplace_back(&aux_hud.tag[j].bg_default_colour);
			secondary.emplace_back(&aux_hud.tag[j].bg_disabled_colour);
			tertiary.emplace_back(&aux_hud.tag[j].bg_flash_colour);
			secondary.emplace_back(&aux_hud.tag[j].meter_disabled_colour);
			primary.emplace_back(&aux_hud.tag[j].meter_empty_colour);
			tertiary.emplace_back(&aux_hud.tag[j].meter_flash_colour);
			primary.emplace_back(&aux_hud.tag[j].meter_max_colour);
			primary.emplace_back(&aux_hud.tag[j].meter_min_colour);
		}

		Reflexive<AuxiliaryOverlay>& aux_overlay = (*i)->auxiliary_overlays;

		for(unsigned int j = 0; j < aux_overlay.count; j++) {
			primary.emplace_back(&aux_overlay.tag[j].default_colour);
			secondary.emplace_back(&aux_overlay.tag[j].disabled_colour);
			tertiary.emplace_back(&aux_overlay.tag[j].flashing_colour);
		}
	}
	
	for(auto i = ghud.begin(); i != ghud.end(); i++) {
		primary.emplace_back(&(*i)->bg_default_colour);
		secondary.emplace_back(&(*i)->bg_disabled_colour);
		tertiary.emplace_back(&(*i)->bg_flashing_colour);
		primary.emplace_back(&(*i)->total_grenades_bg_default_colour);
		secondary.emplace_back(&(*i)->total_grenades_bg_disabled_colour);
		tertiary.emplace_back(&(*i)->total_grenades_bg_flashing_colour);
	}

	for(auto i = weapons.begin(); i != weapons.end(); i++) {
		Reflexive<StaticElement>& static_elements = (*i)->static_elements;
		Reflexive<MeterElement>& meter_elements = (*i)->meter_elements;
		Reflexive<NumberElement>& number_elements = (*i)->number_elements;

		for(unsigned int j = 0; j < meter_elements.count; j++) {
			primary.emplace_back(&meter_elements.tag[j].meter_maximum);
			primary.emplace_back(&meter_elements.tag[j].meter_minimum);
			secondary.emplace_back(&meter_elements.tag[j].disabled_colour);
			primary.emplace_back(&meter_elements.tag[j].empty_colour);
			tertiary.emplace_back(&meter_elements.tag[j].flash_colour);
		}

		for(unsigned int j = 0; j < number_elements.count; j++) {
			primary.emplace_back(&number_elements.tag[j].default_colour);
			tertiary.emplace_back(&number_elements.tag[j].flashing_colour);
		}
	
		for(unsigned int j = 0; j < static_elements.count; j++) {
			primary.emplace_back(&static_elements.tag[j].default_colour);
			secondary.emplace_back(&static_elements.tag[j].disabled_colour);
			tertiary.emplace_back(&static_elements.tag[j].flashing_colour);
		}

		Reflexive<Reticle>& reticleMeta = (*i)->crosshairs;

		for(unsigned int j = 0; j < reticleMeta.count; j++) {
			Reflexive<ReticleOverlay>& reticleOverlays = reticleMeta.tag[j].overlays;
			for(unsigned int k = 0; k < reticleOverlays.count; k++) {
				reticle.emplace_back(&reticleOverlays.tag[k].default_colour);
				reticle.emplace_back(&reticleOverlays.tag[k].flashing_colour);
			}
		}

		Reflexive<OverlayElement>& overlayMeta = (*i)->overlay_elements;
		
		for(unsigned int j = 0; j < overlayMeta.count; j++) {
			Reflexive<Overlay>& overlays = overlayMeta.tag[j].overlays;
			for(unsigned int k = 0; k < overlays.count; k++) {
				primary.emplace_back(&overlays.tag[k].default_colour);
				secondary.emplace_back(&overlays.tag[k].disabled_colour);
				tertiary.emplace_back(&overlays.tag[k].flashing_colour);
			}
		}
	}

	groups[PRIMARY_COLOUR] = primary;
	groups[SECONDARY_COLOUR] = secondary;
	groups[TERTIARY_COLOUR] = tertiary;
	groups[RETICLE_COLOUR] = reticle;
}

void colour(const HSVColour& offsets, ToggleState state) {
	if(groups.find(state) != groups.end()) {
		std::vector<ColourInformation*> matches = groups[state];
		for(auto i = matches.begin(); i != matches.end(); i++) {
			recolour(*(*i), offsets);
		}
	}
}

HSVColour rgb2hsv(RGBColour& rgb) {
	double min = min(rgb.blue, min(rgb.green, rgb.red));
	double max = max(rgb.blue, max(rgb.green, rgb.red));

	HSVColour hsv = {0.0};
	hsv.value = max;

	if(hsv.value == 0.0) {
		return hsv;
	}

	rgb.red /= hsv.value;
	rgb.green /= hsv.value;
	rgb.blue /= hsv.value;

	min = min(rgb.blue, min(rgb.green, rgb.red));
	max = max(rgb.blue, max(rgb.green, rgb.red));

	hsv.saturation = max - min;

	if(hsv.saturation == 0.0) {
		return hsv;
	}

	rgb.red = (rgb.red - min)/(max - min);
	rgb.green = (rgb.green - min)/(max - min);
	rgb.blue = (rgb.blue - min)/(max - min);
	min = min(rgb.blue, min(rgb.green, rgb.red));
	max = max(rgb.blue, max(rgb.green, rgb.red));

	if(max == rgb.red) {
		hsv.hue = 60.0 * (rgb.green - rgb.blue);
		if(hsv.hue < 0.0) {
			hsv.hue += 360.0;
		}
	} else if(max == rgb.green) {
		hsv.hue = 120.0 + 60.0 * (rgb.blue - rgb.red);
	} else {
		hsv.hue = 240.0 + 60.0 * (rgb.red - rgb.green);
	}

	return hsv;
}

RGBColour hsv2rgb(HSVColour& hsv) {
	double	hh, p, q, t, ff;
    long i;
	RGBColour rgb = {0};

	if(hsv.saturation == 0.0) {
		rgb.red = hsv.value;
        rgb.green = hsv.value;
        rgb.blue = hsv.value;
        return rgb;
    }

	hh = fmod(hsv.hue, 360.0);
   
	if(hh >= 360.0) {
		hh = 0.0;
	}
    
	hh /= 60.0;
    i = static_cast<long>(hh);
    ff = hh - i;
	p = hsv.value * (1.0 - hsv.saturation);
	q = hsv.value * (1.0 - (hsv.saturation * ff));
	t = hsv.value * (1.0 - (hsv.saturation * (1.0 - ff)));

    switch(i) {
    case 0:
		rgb.red = hsv.value;
		rgb.green = t;
		rgb.blue = p;
        break;
    case 1:
        rgb.red = q;
		rgb.green = hsv.value;
		rgb.blue = p;
        break;
    case 2:
        rgb.red = p;
        rgb.green = hsv.value;
        rgb.blue = t;
        break;

    case 3:
        rgb.red = p;
        rgb.green = q;
        rgb.blue = hsv.value;
        break;
    case 4:
        rgb.red = t;
        rgb.green = p;
        rgb.blue = hsv.value;
        break;
    case 5:
    default:
        rgb.red = hsv.value;
        rgb.green = p;
        rgb.blue = q;
        break;
    }

    return rgb;
}

}