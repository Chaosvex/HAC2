#include "OpticHooks.h"
#include "PatchGroup.h"
#include "Patcher.h"
#include "Codefinder.h"
#include <cstdint>

namespace Optic { namespace Hooks {

void SpreeHandlerCodecave();
std::unique_ptr<PatchGroup> patches = nullptr;
std::uintptr_t ContinueSpreeString;
std::uintptr_t SkipSpreeString;

void install() {
	patches = std::unique_ptr<PatchGroup>(new PatchGroup());
	short signature[] = {0xC6, 0x44, 0x24, 0x0F, 0x00, 0x8B, 0x94, 0x24, 0xA8, 0x00, 0x00, 0x00};
	std::uintptr_t addr = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(addr == NULL) {
		throw HookException("Spree mute hook failed!");
	}

	SkipSpreeString = addr;

	short pattern[] = {0x8B, 0x84, 0x24, 0xA0, 0x00, 0x00, 0x00, 0xFF, 0x24, 0x85};
	patches->add(new CaveHook(pattern, sizeof(pattern) / 2, 0, SpreeHandlerCodecave, CaveHook::NAKED, &ContinueSpreeString));

	if(!patches->install()) {
		patches.reset();
		throw HookException("Spree mute hook failed!");
	}
}

void uninstall() {
	patches.reset();
}

void __declspec(naked) SpreeHandlerCodecave() {
	__asm {
		mov eax, dword ptr ss:[esp+0x0a0]
		cmp eax, 7
		jb Back
		cmp eax, 18
		ja Back
		cmp eax, 8
		je Back
		cmp eax, 13
		je Back
		jmp SkipSpreeString
	Back:
		jmp ContinueSpreeString
	}
}

}};