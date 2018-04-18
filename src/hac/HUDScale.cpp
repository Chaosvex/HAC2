#include "HUDScale.h"
#include "EnginePointers.h"
#include "EngineTypes.h"
#include "DebugHelper.h"
#include "EngineFunctions.h"
#include "Preferences.h"
#include "EngineFunctions.h"
#include "Language.h"
#include "KeyMappings.h"
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <vector>
#include <map>

using namespace keymappings;
using namespace EngineTypes;

std::uintptr_t ContinueCounterRender;

float x_scale = 1.0f;
std::vector<std::uint16_t*> pGametypeIconOffsets;
std::vector<float*> pRadarMask;
std::uint16_t* pGametypeIconBGXPos;
float* pGametypeIconBGWidth;

void __declspec(naked) HUDCounterScale() {
	__asm {
		fld dword ptr [esp + 0x3c]
		push x_scale
		fmul dword ptr [esp]
		fstp [esp]
		mov eax, [esp]
		add esp, 4
		mov edx, dword ptr [esp + 0x3c]
		jmp ContinueCounterRender
	}
}

namespace HUDScale {

std::vector<HUDCounter> originalText;
std::vector<float> originalRadarMask;
std::vector<std::uint16_t> originalIconOffsets;
std::map<LayoutElement*, LayoutElement> offsets;
bool toggled = false, fixed = false;
float scale_factor;
void scale(float x, float y);

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

template<typename T>
void scale(Reflexive<T>& reflexive, double x, double y) {
	for(unsigned int i = 0; i < reflexive.count; i++) {
		scale(reflexive.tag[i].layout, x, y);
	}
}

void reset(bool bg_reset) {
	x_scale = 1.0f;

	if(bg_reset) {
		*pGametypeIconBGXPos = 445;
		*pGametypeIconBGWidth = 1.0f;
	}

	for(size_t i = 0, j = originalIconOffsets.size(); i != j; i++) {
		*pGametypeIconOffsets[i] = originalIconOffsets[i];
	}
	
	for(size_t i = 0, j = originalRadarMask.size(); i != j; i++) {
		*pRadarMask[i] = originalRadarMask[i];
	}

	for(auto i = offsets.begin(); i != offsets.end(); i++) {
		*(i)->first = i->second;
	}

	std::vector<HUDCounter*> hudh = tagSearch<HUDCounter>(HUDH);

	for(size_t i = 0, j = originalText.size(); i != j; i++) {
		hudh[i]->screenDigitWidth = originalText[i].screenDigitWidth;
		hudh[i]->decimalPointWidth = originalText[i].decimalPointWidth;
		hudh[i]->colonWidth = originalText[i].colonWidth;
	}

	offsets.clear();
}

void ratioFix(bool command) {
	if(command && !offsets.empty()) {
		return;
	}
	
	originalText.clear();
	offsets.clear();
	double aspect = (640.0 / 480.0) / (pResolution->width / static_cast<float>(pResolution->height)); 
	scale(aspect, 1.0f);
	scale_factor = 1.0f;
	x_scale = aspect;
}

void adjust(BYTE *pMouseBuffer) {
	if(toggled) {
		bool print = false;
		if(pMouseBuffer[MOUSE_UP]) {
			pMouseBuffer[MOUSE_UP] = 0;
			if(scale_factor < 1.99f) {	
				scale_factor *= 1.01f;
				scale(1.01f, 1.01f);
				print = true;
			}
		} else if(pMouseBuffer[MOUSE_DOWN]) {
			pMouseBuffer[MOUSE_DOWN] = 0;
			if(scale_factor > 0.51f) {
				scale_factor *= 0.99f;
				scale(0.99f, 0.99f);
				print = true;
			}
		}

		if(print) {
			char buffer[64];
			sprintf_s(buffer, 64, Language::GetString(HUD_SCALE).c_str(), static_cast<int>(scale_factor * 100));
			ExecuteCommand("hud_clear_messages");
			HUDMessage(buffer);
		}
	}
}

void toggle() {
	if(!toggled) {
		HUDMessage(Language::GetString(HUD_SCALE_ADJUST));
	} else {
		char buffer[64];
		sprintf_s(buffer, 64, Language::GetString(HUD_SCALE_SET).c_str(), static_cast<int>(scale_factor * 100));
		HUDMessage(buffer);
		//Preferences::add("hud", scale_factor);
	}

	toggled = !toggled;
}

double round(double number) {
    return number < 0.0? ceil(number - 0.5) : floor(number + 0.5);
}

void scale(LayoutElement& elem, float x, float y, bool truncate = false) {
	if(offsets.find(&elem) == offsets.end()) {
		offsets[&elem] = elem;
	}

	if(truncate) {
		elem.offset.x = static_cast<int16_t>(elem.offset.x * x);
		elem.offset.y = static_cast<int16_t>(elem.offset.y * y);
	} else {
		elem.offset.x = static_cast<int16_t>(round(elem.offset.x * x));
		elem.offset.y = static_cast<int16_t>(round(elem.offset.y * y));
	}

	elem.scale.x *= x;
	elem.scale.y *= y;
}

void scale(float x, float y) {
	std::vector<UnitHUD*> hud = tagSearch<UnitHUD>(UNHI);
	std::vector<WeaponHUD*> weapons = tagSearch<WeaponHUD>(WPHI);
	std::vector<GrenadeHUD*> ghud = tagSearch<GrenadeHUD>(GRHI);
	std::vector<HUDCounter*> hudh = tagSearch<HUDCounter>(HUDH);

	*pGametypeIconBGWidth = 1 / x;
	*pGametypeIconBGXPos = static_cast<std::uint16_t>(443 + ((640.f - 445) / (1.f / (1 - x))));

	if(originalRadarMask.empty()) {
		for(auto i = pRadarMask.begin(); i != pRadarMask.end(); ++i) {
			originalRadarMask.emplace_back(**i);
		}
	}

	if(originalIconOffsets.empty()) {
		for(auto i = pGametypeIconOffsets.begin(); i != pGametypeIconOffsets.end(); ++i) {
			originalIconOffsets.emplace_back(**i);
		}
	}

	for(size_t i = 0, j = pGametypeIconOffsets.size(); i != j; ++i) {
		*pGametypeIconOffsets[i] = static_cast<std::uint16_t>
			((originalIconOffsets[i] - 2) + ((640 - originalIconOffsets[i]) / (1.0f / (1 - x))));
	}
	
	for(size_t i = 0, j = pRadarMask.size(); i != j; ++i) {
		//*pRadarMask[i] = originalRadarMask[i] * x;
	}

	for(auto i = hudh.begin(); i != hudh.end(); ++i) {
		originalText.emplace_back(**i);
		(*i)->screenDigitWidth = static_cast<int8_t>(ceil((*i)->screenDigitWidth * x));
		(*i)->decimalPointWidth = static_cast<int8_t>(ceil((*i)->decimalPointWidth * x));
		(*i)->colonWidth = static_cast<int8_t>(ceil((*i)->colonWidth * x));
	}

	for(auto i = hud.begin(); i != hud.end(); ++i) {
		scale((*i)->shield_panel_bg_layout, x, y);
		scale((*i)->health_panel_bg_layout, x, y, true);
		scale((*i)->health_panel_meter_layout, x, y);
		scale((*i)->hud_bg_layout, x, y);
		scale((*i)->motion_sensor_centre_layout, x, y);
		scale((*i)->motion_sensor_foreground_layout, x, y);
		scale((*i)->motion_sensor_bg_layout, x, y);
		scale((*i)->shield_panel_meter_layout, x, y);
		scale((*i)->auxiliary_overlays, x, y);

		Reflexive<AuxiliaryHUDMeter>& aux_hud = (*i)->auxiliary_hud_meters;
		
		for(std::uint32_t j = 0; j < aux_hud.count; ++j) {
			scale(aux_hud.tag[j].bg_layout, x, y);
			scale(aux_hud.tag[j].meter_layout, x, y);
		}
	}
	
	for(auto i = ghud.begin(); i != ghud.end(); ++i) {
		scale((*i)->bg, x, y);
		scale((*i)->total_grenades_bg, x, y);
		scale((*i)->total_grenade_numbers, x, y);
		scale((*i)->overlays, x, y);
	}

	for(auto i = weapons.begin(); i != weapons.end(); ++i) {
		scale((*i)->static_elements, x, y);
		scale((*i)->meter_elements, x, y);
		scale((*i)->number_elements, x, y);

		Reflexive<Reticle> reticleMeta = (*i)->crosshairs;
		Reflexive<OverlayElement> overlayMeta = (*i)->overlay_elements;

		for(std::uint32_t j = 0; j < reticleMeta.count; ++j) {
			scale(reticleMeta.tag[j].overlays, x, y);
		}
		
		for(std::uint32_t j = 0; j < overlayMeta.count; ++j) {
			scale((*i)->overlay_elements.tag[j].overlays, x, y);
		}
	}
}

}