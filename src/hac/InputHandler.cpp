#include "InputHandler.h"
#include "KeyMappings.h"
#include "FoVControl.h"
#include "Sightjacker.h"
#include "Bookmarker.h"
#include "EngineState.h"
#include "EnginePointers.h"
#include "Shared.h"
#include "EngineFunctions.h"
#include "console.h"
#include "ServerInformation.h"
#include "ServerQuery.h"
#include "Language.h"
#include "HUDScale.h"
#include "HUDColour.h"
#include "Eggs.h"
#include "ScoreboardTimer.h"
#include "DisplayModes.h"
#include <process.h>

using namespace keymappings;
using namespace EngineState;
using namespace EngineTypes;

BYTE* pKeyboardBuffer;
BYTE* pMouseBuffer;

void keyboardRead() {
	if(pKeyboardBuffer[KEY_PLUS] || pKeyboardBuffer[KEY_MINUS]) {
		SightJacker::switchPlayer(pMouseBuffer, pKeyboardBuffer);
	} else if(pKeyboardBuffer[LEFT_SHIFT] && pKeyboardBuffer[KEY_F6] == 1) {
		FoV::reset();
	} else if(pKeyboardBuffer[LEFT_SHIFT] && pKeyboardBuffer[KEY_F8] == 1) {
		HUDColour::reset();
	} else if(pKeyboardBuffer[KEY_F4] == 1) { //Bookmark
		Bookmarker bookmark;
		bookmark.bookmark(serverAddress, reconnectPassword, serverName);
	} else if(pKeyboardBuffer[KEY_F5] == 1) { //Bookmark recall
		_beginthread(bookmarkDisplayAsync, 0, NULL);
	} else if(pKeyboardBuffer[KEY_F6] == 1) { //FoV
		FoV::toggle();
	} else if(pKeyboardBuffer[KEY_F7] == 1) { //SJ
		SightJacker::toggle();
	} else if(pKeyboardBuffer[KEY_F8] == 1) { //HUD colour
		HUDColour::toggle();
	} else if(pKeyboardBuffer[LEFT_ALT]) { //Bookmark connect
		bookmarkConnect(pKeyboardBuffer);
		if(pKeyboardBuffer[KEY_ENTER] == 1) {
			DisplayModes::toggleDisplayMode();
			pKeyboardBuffer[KEY_ENTER] = 0;
		}
	} else if(pKeyboardBuffer[KEY_ESC]) {
		setState(HALT_DOWNLOAD);
		setState(LEAVE_QUEUE);
	} else if(pKeyboardBuffer[KEY_DELETE]) { //Bookmark delete
		bookmarkRemove(pKeyboardBuffer);
	}
	
	konamiCode(pKeyboardBuffer);
}

void mouseRead() {
	HUDScale::adjust(pMouseBuffer);
	FoV::adjust(pMouseBuffer);
	HUDColour::adjust(pMouseBuffer);
	SightJacker::switchPlayer(pMouseBuffer, pKeyboardBuffer);
}