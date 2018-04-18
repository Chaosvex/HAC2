#include "ScopeBlur.h"
#include "Patcher.h"
#include "console.h"
#include "Preferences.h"
#include <memory>

namespace ScopeBlurFix {

bool enabled = false;
std::unique_ptr<PatchHook> patch = NULL;

void set(bool toggle) {
	toggle? enable() : disable();
}

void enable(){ 
	if(enabled) {
		return;
	}

	enabled = true;

	short signature[] = {0x7A, 0x08, 0xD9, 0x05, -1, -1, -1, -1, 0xEB, -1, 0xD9, 0x05, -1, -1, -1, -1, 0xD8, 0x62, 0x44};
	std::uint8_t replacement[] = {0x90, 0x90};
	patch = std::unique_ptr<PatchHook>(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, 2));
	
	if(!patch->install()) {
		hkDrawText("Unable to disable scope blur!", C_TEXT_RED);
	}
}

void disable() {
	patch.reset();
	enabled = false;
}

};