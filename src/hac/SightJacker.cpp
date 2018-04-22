#include "SightJacker.h"
#include "ConsoleCommands.h"
#include "EnginePointers.h"
#include "EngineFunctions.h"
#include "EngineState.h"
#include "KeyMappings.h"
#include "Language.h"
#include "Direct3D.h"
#include <cstdint>
#include <sstream>

using namespace keymappings;
using namespace EngineTypes;

EngineTypes::CameraControl *pCameraControl;
EngineTypes::PlayerHeader **pPlayerHeader;

enum MODE {
	NEXT_PLAYER, PREVIOUS_PLAYER
};

namespace {

short playerIndex = 0;
std::uint32_t targetIndex = -1;
bool enabled = false;
bool refocus = false;

void printPlayer(WORD objectID);
void nextPlayer(MODE mode);
void setPlayer(std::uint16_t playerIndex);
void previousPlayerIndex();
EngineTypes::Player* fetchPlayer(WORD objectID);

};

bool SightJacker::isEnabled() {
	return enabled;
}

void SightJacker::enable() {
	//No sense in allowing the SJ on the UI
	if(_stricmp(pCurrentMap, "ui") == 0) {
		return;
	}

	HUDMessage(Language::GetString(SJ_ENABLE));
	PlayMPSound(_multiplayer_sound_countdown_timer_end);

	enabled = true;
	nextPlayer(NEXT_PLAYER);
	ExecuteCommand("camera_control 1", true);
}

void SightJacker::disable(bool surpress) {	
	enabled = false;
	
	if(!surpress) {
		ExecuteCommand("camera_control 0", true);
		HUDMessage(Language::GetString(SJ_DISABLE));
		PlayMPSound(_multiplayer_sound_countdown_timer_end);
	}
}

void SightJacker::switchPlayer(const BYTE* pMouseBuffer, BYTE* pKeyboardBuffer) {
	if(enabled) {
		if(pMouseBuffer[SCROLL_WHEEL_UP] || pKeyboardBuffer[KEY_PLUS] == 1) {
			nextPlayer(NEXT_PLAYER);
		} else if(pMouseBuffer[SCROLL_WHEEL_DOWN] || pKeyboardBuffer[KEY_MINUS] == 1) {
			nextPlayer(PREVIOUS_PLAYER);
		}

		/* The game doesn't seem to 'eat' these presses until the next frame,
		   so we do it here to stop the target from switching twice per press */
		pKeyboardBuffer[KEY_PLUS] = 0;
		pKeyboardBuffer[KEY_MINUS] = 0;
	}
}

void SightJacker::toggle() {
	enabled? disable() : enable();
}

void SightJacker::playerDied(std::uint32_t index) {
	if(index == targetIndex) {
		refocus = true;
	}
}

void SightJacker::lost() {
}

void SightJacker::reset() {
}

void SightJacker::draw() {
	if(!enabled) {
		return;
	}

	/*D3DVIEWPORT9 pViewport;
	D3DHook::pDevice->GetViewport(&pViewport);
	D3DRECT rec0 = {pViewport.Width/2 -1, pViewport.Height/2 -1, pViewport.Width/2 +1, pViewport.Height/2 +1};								
	D3DHook::pDevice->Clear(1, &rec0, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 255, 224,  64,  64 ), 0, 0);*/

	if(refocus) {
		ExecuteCommand("hud_clear_messages");
		static bool waitingSpace = false;
		PlayerHeader* header = *pPlayerHeader;
		Player* players = header->firstPlayer;

		if(players[targetIndex].respawn_timer != 0 && !waitingSpace) {
			std::wstringstream countdown;
			countdown << players[targetIndex].name << " respawning in " << floor((players[targetIndex].respawn_timer * (1.0 / 30)));
			HUDMessage(countdown.str());
		} else {
			waitingSpace = true;
			HUDMessage("Waiting for space to clear");
		}
		
		if(players[targetIndex].objectIndex != 0xFFFF) {
			int nindex = -1;
			
			for(std::uint16_t i = header->m_wSlotsTaken-1; i >= targetIndex; i--) {
				if(players[i].objectIndex != 0xFFFF && players[i].objectID != 0xFFFF) {
					nindex++;
				}
			}

			setPlayer(nindex);
			waitingSpace = false;
			ExecuteCommand("hud_clear_messages");
		}
	} else {
		
	}
}

namespace {

void setPlayer(std::uint16_t playerIndex) {
	char buffer[64];
	sprintf_s(buffer, 64, "camera_set_first_person (unit (list_get (players)%i))", playerIndex);
	ExecuteCommand(buffer, true);
	refocus = false;
}

void nextPlayer(MODE mode) {
	PlayerHeader* header = *pPlayerHeader;
	Player* player = header->firstPlayer;
	WORD currID = pCameraControl->objectID;

	for(int i = 0; i < header->m_wMaxSlots; i++) {
		if(mode == NEXT_PLAYER) {
			playerIndex = (playerIndex + 1) % header->m_wMaxSlots;
		} else {
			playerIndex = (playerIndex == 0)? header->m_wMaxSlots - 1 : playerIndex - 1;
		}

		setPlayer(playerIndex);

		if(currID != pCameraControl->objectID) {
			printPlayer(pCameraControl->objectID);
			break;
		}
	}
}

void printPlayer(std::uint16_t objectID) {
	Player* player = fetchPlayer(objectID);

	if(player != NULL) { //should never happen but better safe than sorry
		wchar_t buffer[128];
		swprintf_s(buffer, 128, Language::GetWideString(SJ_SPECTATING).c_str(), player->name);
		HUDMessage(buffer);
	}
}

Player* fetchPlayer(std::uint16_t objectID) {
	PlayerHeader* header = *pPlayerHeader;
	Player* player = header->firstPlayer;

	for(int i = 0; i < header->m_wMaxSlots; ++i, ++player) {
		if(objectID == player->objectID) {
			targetIndex = i;
			return player;
		}
	}

	return NULL;
}

};