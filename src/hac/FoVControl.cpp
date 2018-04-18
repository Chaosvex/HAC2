#include "FoVControl.h"
#include "EngineFunctions.h"
#include "Preferences.h"
#include "KeyMappings.h"
#include "Language.h"
#include "Shared.h"
#include "EventDispatcher.h"
#include "Events.h"
#include <memory>
#include <stdio.h>

using namespace keymappings;
using namespace FoV;

const float M_PI = 3.14159265358979323846f;

float* fovScaler;
std::uintptr_t ContinueFoV;

namespace {

float scale(float desired);
float radiansToDegrees(float d);
float degreesToRadians(float r);
bool fovToggle = false;
const float DEFAULT_VIEW = 1.221730471f;
const float DEFAULT_SCALE = 0.8500000238f;
float fieldOfView = DEFAULT_VIEW;

}

void FoV::reset() {
	fieldOfView = DEFAULT_VIEW;
	HUDMessage(Language::GetWideString(FOV_RESET).c_str());
	Preferences::add("fov", get());
}

void _declspec(naked) FoV::redirect() {
	__asm {
		pushfd
		pushad
		push edx
		call scale
		fstp [ebx+0xAC]
		add esp, 4
		popad
		popfd
		jmp ContinueFoV
	}
}

void FoV::set(float degrees) {
	fieldOfView = degreesToRadians(degrees);
}

float FoV::get() {
	return radiansToDegrees(fieldOfView);
}

void FoV::toggle() {
	if(!fovToggle) {
		HUDMessage(Language::GetWideString(FOV_ADJUST).c_str());
	} else {
		wchar_t buffer[128];
		swprintf_s(buffer, 128, Language::GetWideString(FOV_SET).c_str(), get());
		HUDMessage(buffer);
		Preferences::add("fov", get());
		dispatcher->enqueue(std::make_shared<Event>(FOV_ADJUSTED));
	}

	fovToggle = !fovToggle;
}

void FoV::adjust(BYTE *pMouseBuffer) {
	if(fovToggle) {
		bool print = false;
		if(pMouseBuffer[MOUSE_UP]) {
			pMouseBuffer[MOUSE_UP] = 0;
			if(get() < 180) {			
				set(get() + 1);
				print = true;
			}
		} else if(pMouseBuffer[MOUSE_DOWN]) {
			pMouseBuffer[MOUSE_DOWN] = 0;
			if(get() > 1) {
				set(get() - 1);
				print = true;
			}
		}

		if(print) {
			wchar_t buffer[32];
			swprintf_s(buffer, 32, Language::GetWideString(FOV).c_str(), get());
			ExecuteCommand("hud_clear_messages");
			HUDMessage(buffer);
		}
	}
}

namespace {

float radiansToDegrees(float d) {
	return d * (float)(180.0f / M_PI);
}

float degreesToRadians(float r) {
	return r * (float)(M_PI / 180.0f);
}

float scale(float desired) {
	float scale = desired / DEFAULT_VIEW;
	float final = fieldOfView;

	if(scale < 1.f) {
		final *= scale;
	}

	return final;
}

}