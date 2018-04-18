#include "EngineState.h"
#include <Windows.h>
#include <cstdint>

std::uintptr_t BackToHalo;

namespace {
	int index;
	std::uint32_t opacity;
	int address;
	std::uint32_t lastUpdate[100] = {0};
	unsigned int delay;
}

/*
 * Halo's console text fade speed is dependent on framerate.
 * On each frame, the opacity of each console message is decreased.
 * This function simply regulates the fade speed by means of undoing
 * any opacity change until a certain amount of time has passed.
 */
void __declspec(naked) ConsoleFadeFixCC() {
	__asm {
		pushad
		pushfd
		mov index, eax
		imul eax,eax,0x124
		lea edi, [eax+esi+0x120]
		mov address, edi
		mov ecx, [eax+esi+0x120]
		mov opacity, ecx
	}

	/* 
	 * Pausing the console output entirely rather than slowing the rate of fade
	 * isn't [easily] possible as the engine's console output function is 
	 * asynchronous. Outputting text followed by an immediate pause could
	 * lead to the text never being displayed.
	 */
	delay = EngineState::checkState(EngineState::SLOW_OUTPUT)? 120 : 30;

	if((GetTickCount() - lastUpdate[index]) < delay && opacity) {
		__asm {
			mov edi, address
			dec [edi]
		}
	} else {
		lastUpdate[index] = GetTickCount();
	}

	__asm {
		popfd
		popad
		imul eax, eax, 0x124
		jmp BackToHalo
	}
}