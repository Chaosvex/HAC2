#include "SharedCaves.h"
#include "MapDownload.h"
#include "Preferences.h"
#include "SlotQueue.h"
#include "EngineState.h"
#include "Sightjacker.h"
#include "ExFlagHandler.h"
#include "EngineFunctions.h"
#include "EnginePointers.h"
#include "DisplayUpdater.h"
#include "Shared.h"
#include "versionchanger.h"
#include "MapControl.h"
#include "ServerQuery.h"
#include "TimerManager.h"
#include "EventDispatcher.h"
#include "ServerInformation.h"
#include "CustomChat.h"
#include "HUDScale.h"
#include "NetEvents.h"
#include "Events.h"
#include "HUDColour.h"
#include "CustomChat.h"
#include "Optic.h"
#include "ScoreboardTimer.h"
#include "DiscordHandlers.h"
#include <discord-rpc.h>
#include <string>
#include <memory>
#include <cstdint>

using namespace EngineTypes;

std::uintptr_t BackToConnecting, TestCont, ContinuePlaySound, ContinuePlayerEvent, BackToJoining, BackToEngineTick;

const int QUERY_BUFFER_LENGTH = 4096;

void drawDistanceFix() {
	if(_stricmp(pCurrentMap, "hugeass") == 0 || _stricmp(pCurrentMap, "extinction") == 0) {
		DrawDistance(4096.f);
	} else {
		float distance = Preferences::find<float>("draw_distance", 1024.0f);
		DrawDistance(distance);
	}
}

/*
 * When a player joins a server, we save the password argument so it can be used
 * if we need to force a reconnect after a map download.
 */
void SavePassword(wchar_t *password) {
	if(password != NULL && wcscmp(password, L"\0") != 0) {
		wcscpy_s(reconnectPassword, PASSWORD_LENGTH, password);
	} else {
		wcscpy_s(reconnectPassword, PASSWORD_LENGTH, L"''");
	}
}

/*
 * Intercepts the join requests and saves the password for later use.
 * Note: Should probably just save everything here (server info pointer is also
 * pushed on the stack).
 */
void _declspec(naked) SavePasswordStub() {
	__asm {
		mov eax, [esp+0x28]
		push eax
		call SavePassword
		add esp, 4
		jmp BackToJoining
	}
}

void engineTick() {
	scoreboard::on_tick();
}

void resolutionNotify() {
	Chat::resolutionChange();
	
	if(Preferences::find<bool>("hud_ratio_fix", true)) {
		HUDScale::reset();
		HUDScale::ratioFix();
	}

	dispatcher->enqueue(std::make_shared<Event>(RESOLUTION_CHANGE));
}

bool checkConnectSafety() {
	return !(checkState(EngineState::DOWNLOADING) || checkState(EngineState::QUEUED));
}

HANDLE serverJoinCritSection;

void joinCaveLock() {
	WaitForSingleObject(serverJoinCritSection, INFINITE);
}

void joinCaveUnlock() {
	ReleaseSemaphore(serverJoinCritSection, 1, NULL);
}

char queryResponse[QUERY_BUFFER_LENGTH];

bool queryServer(ConnectionDetails* details, char* buffer) {
	serverInfo.type = ServerDetails::Type::STOCK; // hack

	ServerInfo info(details);
	lastServer = details;
	ServerQuery query(info);
	return query.query(queryResponse, QUERY_BUFFER_LENGTH);
}

/*
 * The codecave for intercepting server joins.
 */
void* pServerInfo;
bool (*slotQueue)(const char* response, const EngineTypes::ConnectionDetails* info) = fullQueue;
void _declspec(naked) serverJoinCave() {
	__asm {
		//Hook code
		pushfd
		pushad
		call joinCaveLock
		call checkConnectSafety
		test al, al
		jz Abort
		mov pServerInfo, ebx;
		push ebx
		call queryServer
		add esp, 4
		test al, al
		jz Continue

		//Update client version and check to see if the map exists
		lea eax, queryResponse
		push eax
		push eax
		push eax
		call versionCheck
		add esp, 4
		call setExtendedFlags
		add esp, 4
		call mapCheck
		add esp, 4
		test al, al
		jz Download
		lea eax, queryResponse
		push pServerInfo
		push eax
		call slotQueue
		add esp, 8
		test al, al
		jnz Abort

		//Original code
		Continue:
		call joinCaveUnlock
		popad
		popfd
		mov ecx, 0x7FF
		jmp BackToConnecting

		//Kick back to menu and initiate map download
		Download:
		lea eax, queryResponse
		push eax
		call beginDownload
		add esp, 4
		
		Abort:
		call joinCaveUnlock
		popad
		popfd
		xor eax, eax
		pop edi
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}

void mapLoadStub() {
	SightJacker::disable(true);
	drawDistanceFix();
#ifdef _DEBUG
	beginTimerAuto();
#endif
}

void delayedInitialisation() {

}

void postMapLoad() {
	if(!Preferences::find<bool>("hud_ratio_fix", true)) {
		HUDScale::reset(false);
	} else {
		HUDScale::ratioFix();
	}

	HUDColour::loadState();
	DisplayUpdater::notify();
	dispatcher->enqueue(std::make_shared<Event>(POST_MAP_LOAD));
	delayedInitialisation();
	NetEvents::sendHandshake();
    DiscordHandlers::presenceUpdate();
}

/*
 * Fixes something. No idea what anymore.
 */
void _declspec(naked) TestCC() {
	__asm{
		pushfd
		test eax, eax
		jz BREAK
		popfd
		mov ecx, 0x0c
		jmp TestCont
		BREAK:
		popfd
		pop ebx
		pop edi
		pop esi
		pop ebp
		add esp, 0x34
		retn
	}
}

//This is in its own function to avoid adding overhead to the
//sound event function (see the generated assembly)
void hitBcast() {
	std::shared_ptr<Event> event = std::make_shared<Event>(HIT_DETECT);
	dispatcher->enqueue(event);
}

void soundEvent(multiplayer_sound index) {
	if(index == _multiplayer_sound_ting) {
		hitBcast();
	}
}

void __declspec(naked) SoundEventStub() {
	__asm {
		pushfd
		pushad
		push esi
		call soundEvent
		add esp, 4
		popad
		popfd
		//Original
		mov byte ptr ss:[esp+4], 0
		jmp ContinuePlaySound
	}
}

/* Replace the crap below when the hook class can fix up jumps */
oOverlayResolve originalOverlayResolve;

void overlayBroadcast(std::uint32_t otype) {
	std::shared_ptr<Event> event;

	switch(otype) {
		case 1:
			event = std::make_shared<Event>(GAME_OVER);
			dispatcher->enqueue(event);
			break;
		case 2:
			event = std::make_shared<Event>(PCR_SHOW);
			dispatcher->enqueue(event);
			break;
		case 3:
			event = std::make_shared<Event>(PCR_EXIT_SHOW);
			dispatcher->enqueue(event);
			break;
	}
}

std::uint32_t overlayTrigger() {
	std::uint32_t* otype;

	__asm {
		mov otype, ecx
	}

	std::uint32_t res = originalOverlayResolve();
	//needs another function for the sake of delicate stack state
	overlayBroadcast(*otype);

	return res;
}

void playerEvent(std::uint32_t local, std::uint32_t victim, std::uint32_t killer, GAME_EVENTS gEvent) {
	std::shared_ptr<Event> event;
	bool broadcast = true, reverse = false;

	local = (local | 0xFFFF0000) ^ 0xFFFF0000;
	victim = (victim | 0xFFFF0000) ^ 0xFFFF0000;
	killer = (killer | 0xFFFF0000) ^ 0xFFFF0000;

	if(gEvent >= FLAG_CAPTURED && gEvent <= ENEMY_FLAG_IDLE_RETURNED) {
		event = std::make_shared<CTFEvent>(CTFEvent(gEvent, victim, killer, local, GetTickCount()));	
	} else if(gEvent == FALLING_DEATH || gEvent == GUARDIAN_KILL || gEvent == VEHICLE_KILL || gEvent == SUICIDE) {
		SightJacker::playerDied(killer);
		event = std::make_shared<PlayerEnvironmentalDeath>(gEvent, killer, local, GetTickCount());	
	} else if(gEvent == PLAYER_KILL || gEvent == BETRAYED) {
		SightJacker::playerDied(victim);
		event = std::make_shared<PlayerKilled>(gEvent, victim, killer, local, GetTickCount());	
	} else if(gEvent == LOCAL_KILLED_PLAYER) {
		SightJacker::playerDied(killer);
		event = std::make_shared<PlayerKilled>(gEvent, killer, victim, local, GetTickCount());	
	} else {
		broadcast = false;
	}

	/*stringstream b;
	b << "#" << gEvent;
	hkDrawText(b.str().c_str(), C_TEXT_RED);*/

	if(broadcast) {
		dispatcher->enqueue(event);
	}
}

void __declspec(naked) PlayerEventStub() {
	__asm {
		push edx //event type
		push ecx //killer ID
		push eax //victim ID
		push edi //local player ID
		call playerEvent
		add esp, 0x10
		jmp ContinuePlayerEvent
	}
}