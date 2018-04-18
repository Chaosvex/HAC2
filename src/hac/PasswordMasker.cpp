#include "PasswordMasker.h"
#include "trex\TRexpp.h"
#include <memory>
#include <Windows.h>

std::uintptr_t ContinueTerminalInput;

namespace {
	char passwordBuff[255];
	char nBuff[255];
	int passwordBuffPos = 0;
	int noMatchPos = 0;
	char nextChar;
}

char PasswordMasker(char* buffer, char currChar, int position) {
	memcpy_s(buffer + position, 255, &currChar, 1);
	const TRexChar *error = NULL;
	TRex *x = trex_compile(_TREXC("login (.+) (.*)"), &error);
	
	if(trex_match(x, buffer) == TRex_True) {
		int buffLoc = (position - noMatchPos) - 1;
		passwordBuff[buffLoc] = currChar;
		passwordBuff[buffLoc + 1] = '\0';
		currChar = '*';
	} else {
		noMatchPos = position;
		passwordBuffPos = 0;
	}

	return currChar;
}

void _declspec(naked) PasswordMaskerStub() {
	__asm {
		//Hook
		pushfd
		pushad
		push ecx
		push eax
		push edx
		call PasswordMasker
		mov byte ptr [nextChar], al
		add esp, 0x0C
		popad
		popfd
		mov al, byte ptr [nextChar]
		
		//Original
		mov [ecx+edx], al
		add esp, 0x0C
		jmp ContinueTerminalInput
	}
}