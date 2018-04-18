#include "MotdRedirect.h"
#include <cstdint>

std::uintptr_t ContinueMotdLoad;

namespace {
	const char* motdURL = "http://maps.haloanticheat.com/motd.txt";
}

void _declspec(naked) MotdRedirect() {
	__asm {
		mov eax, motdURL
		push eax
		xor edx, edx
		jmp ContinueMotdLoad;
	}
}