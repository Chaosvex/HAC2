#include "Eggs.h"
#include "KeyMappings.h"
#include "Shared.h"
#include "console.h"
#include "EngineFunctions.h"
#include <stdio.h>
#include <Windows.h>

using namespace keymappings;

namespace {

BYTE konamiPress(int progress, const BYTE* pKeyboardBuffer) {
	BYTE buffer = 0;

	switch(progress) {
	case 0:
	case 1:
		buffer = pKeyboardBuffer[ARROW_UP];
		break;
	case 2:
	case 3:
		buffer = pKeyboardBuffer[ARROW_DOWN];
		break;
	case 4:
		buffer = pKeyboardBuffer[ARROW_LEFT];
		break;
	case 5:
		buffer = pKeyboardBuffer[ARROW_RIGHT];
		break;
	case 6:
		buffer = pKeyboardBuffer[ARROW_LEFT];
		break;
	case 7:
		buffer = pKeyboardBuffer[ARROW_RIGHT];
		break;
	case 8:
		buffer = pKeyboardBuffer[KEY_B];
		break;
	case 9:
		buffer = pKeyboardBuffer[KEY_A];
		break;
	}

	return buffer;
}

}

void konamiCode(const BYTE* pKeyboardBuffer) {
	static int progress = 0;
	BYTE buffer = konamiPress(progress, pKeyboardBuffer);
	BYTE prevBuffer = konamiPress(progress - 1, pKeyboardBuffer);

	if(buffer == 1) {
		progress++;
	} else if(prevBuffer <= 1) {
		progress = 0;
	}

	if(progress == 10) {
		attentionBox("d");
		progress = 0;
	}
}

void __cdecl printEgg(void*) {
	char buffer[50];
	srand(GetTickCount());

	for(int i = 0; i <= 100; i += rand() % 6 + 2) {
		sprintf_s(buffer, "Formatting drive: %3d percent completed", i);
		Beep(1000, 100);
		hkDrawText(buffer, C_TEXT_YELLOW);
		Sleep(500);
	}

	for(int i = 0; i < 3; i++) Beep(2000, 200);
	hkDrawText("Drive format completed!", C_TEXT_GREEN);

	//Final Fantasy
	Beep(987, 53);
	Beep(987, 53);
	Beep(987, 53);
	Beep(987, 428);
	Beep(784, 428);
	Beep(880, 428);
	Beep(987, 107);
	Beep(880, 107);
	Beep(987, 857);

	Sleep(1000);

	//Axel F
	Beep(676,460);
	Beep(804,340);
	Beep(676,230);
	Beep(676,110);
	Beep(902,230);
	Beep(676,230);
	Beep(602,230);
	Beep(676,460);
	Beep(1012,340);
	Beep(676,230);
	Beep(676,110);
	Beep(1071,230);
	Beep(1012,230);
	Beep(804,230);
	Beep(676,230);
	Beep(1012,230);
	Beep(1351,230);
	Beep(676,110);
	Beep(602,230);
	Beep(602,110);
	Beep(506,230);
	Beep(758,230);
	Beep(676,460);
	Beep(676,460);

	//Tetris
	Beep(1320,500);
	Beep(990,250);
	Beep(1056,250);
	Beep(1188,250);
	Beep(1320,125);
	Beep(1188,125);
	Beep(1056,250);
	Beep(990,250);
	Beep(880,500);
	Beep(880,250);
	Beep(1056,250);
	Beep(1320,500);
	Beep(1188,250);
	Beep(1056,250);
	Beep(990,750);
	Beep(1056,250);
	Beep(1188,500);
	Beep(1320,500);
	Beep(1056,500);
	Beep(880,500);
	Beep(880,500);
	Sleep(250);
	Beep(1188,500);
	Beep(1408,250);
	Beep(1760,500);
	Beep(1584,250);
	Beep(1408,250);
	Beep(1320,750);
	Beep(1056,250);
	Beep(1320,500);
	Beep(1188,250);
	Beep(1056,250);
	Beep(990,500);
	Beep(990,250);
	Beep(1056,250);
	Beep(1188,500);
	Beep(1320,500);
	Beep(1056,500);
	Beep(880,500);
	Beep(880,500);

	//Mario
	Beep(659, 125);
	Beep(659, 125);
	Sleep(125);
	Beep(659, 125);
	Sleep(167);
	Beep(523, 125);
	Beep(659, 125);
	Sleep(125);
	Beep(784, 125);
	Sleep(375);
	Beep(392, 125);
}
