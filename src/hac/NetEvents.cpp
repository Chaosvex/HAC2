#include "NetEvents.h"
#include "NetEventPackets.h"
#include "console.h"
#include "EnginePointers.h"
#include "DebugHelper.h"
#include "EngineFunctions.h"
#include "EngineState.h"
#include "Shared.h"
#include "Sightjacker.h"
#include "ServerInformation.h"
#include "DiscordHandlers.h"
#include "HUDColour.h"
#include <discord-rpc.h>
#include <ctime>
#include <sstream>

namespace NetEvents {

std::uint32_t BackToNetHandler;
std::uint32_t NetworkEventPacket;

void gravityModify(DSData<float>* packet) {
	*gravity = packet->data;
}

void gamespeed(DSData<float>* packet) {
	if(packet->data == 0.0f && *tickrate != 0.0f) {
		hkDrawText("Game paused by server", C_TEXT_GREEN);
		PlayMPSound(EngineTypes::multiplayer_sound::_multiplayer_sound_countdown_timer);
	}

	if(*tickrate == 0.0f && packet->data != 0.0f) {
		hkDrawText("Game resumed", C_TEXT_GREEN);
		PlayMPSound(EngineTypes::multiplayer_sound::_multiplayer_sound_countdown_timer_end);
	}

	*tickrate = packet->data;
}

void handshake(DSHandshake* packet) {
	std::stringstream version;
	
	if(packet->app == AppID::SAPP) {
		serverInfo.type == ServerDetails::Type::SAPP;
		serverInfo.mod_version = -1; // hack
	}
}

void mtvSync(DSData<bool>* packet) {
	ToggleMTV(packet->data);
}

void sightjackerSync(DSData<bool>* packet) {
	if(packet->data && !checkState(EngineState::SCRIM_MODE)) {
		HUDMessage("Sightjacking has been disabled by the server");
		PlayMPSound(EngineTypes::multiplayer_sound::_multiplayer_sound_countdown_timer);
		setState(EngineState::SCRIM_MODE);
		SightJacker::disable();
	} else if(!packet->data && checkState(EngineState::SCRIM_MODE)) {
		HUDMessage("Sightjacking has been enabled by the server");
		PlayMPSound(EngineTypes::multiplayer_sound::_multiplayer_sound_countdown_timer_end);
		clearState(EngineState::SCRIM_MODE);
	}
}

void setTimeleft(DSData<std::int32_t >* packet) {
	timeleft = packet->data;
    DiscordHandlers::presenceUpdate();
}

void dataSyncEvent(NetworkEventBase* packet) {
	switch(packet->opcode) {
		case DataSyncOpcodes::DS_HANDSHAKE:
			hkDrawText("Handshake", C_TEXT_GREEN);
			handshake(static_cast<DSHandshake*>(packet));
			break;
		case DataSyncOpcodes::DS_GAMESPEED:
			hkDrawText("Game speed", C_TEXT_GREEN);
			gamespeed(static_cast<DSData<float>*>(packet));
			break;
		case DataSyncOpcodes::DS_MTV:
			hkDrawText("MTV", C_TEXT_GREEN);
			mtvSync(static_cast<DSData<bool>*>(packet));
			break;
		case DataSyncOpcodes::DS_SJ:
			hkDrawText("SJ", C_TEXT_GREEN);
			sightjackerSync(static_cast<DSData<bool>*>(packet));
			break;
		case DataSyncOpcodes::DS_ZOMBIES:
			// ??
			break;
		case DataSyncOpcodes::DS_TIMELEFT:
			hkDrawText("Time left", C_TEXT_GREEN);
			setTimeleft(static_cast<DSData<std::int32_t>*>(packet));
			break;
		case DataSyncOpcodes::DS_GRAVITY:
			hkDrawText("Gravity - was an OK film", C_TEXT_GREEN);
			//gravityModify(static_cast<DSData<float>*>(packet));
			break;
	}
}

void medalEvent(NetworkEventBase* packet) {
}

void updateHudColour(const HUDColourData* colour, HUDColour::ToggleState state) {
	HUDColour::RGBColour rgb;
	rgb.red = colour->r;
	rgb.blue = colour->b;
	rgb.green = colour->g;

	auto offsets = HUDColour::rgb2hsv(rgb);
	HUDColour::colour(offsets, state);
}

void hudControl(NetworkEventBase* packet) {
	switch(packet->opcode) {
		case NE_HUD_ColourType::NE_HUD_Colour_Primary:
			hkDrawText("NE_HUD_Colour_Primary", C_TEXT_GREEN);
			updateHudColour(static_cast<HUDColourData*>(packet), HUDColour::ToggleState::PRIMARY_COLOUR);
			break;
		case NE_HUD_ColourType::NE_HUD_Colour_Reticle:
			hkDrawText("NE_HUD_Colour_Reticle", C_TEXT_GREEN);
			updateHudColour(static_cast<HUDColourData*>(packet), HUDColour::ToggleState::RETICLE_COLOUR);
			break;
		case NE_HUD_ColourType::NE_HUD_Colour_Secondary:
			hkDrawText("NE_HUD_Colour_Secondary", C_TEXT_GREEN);
			updateHudColour(static_cast<HUDColourData*>(packet), HUDColour::ToggleState::SECONDARY_COLOUR);
			break;
		case NE_HUD_ColourType::NE_HUD_Colour_Tertiary:
			hkDrawText("NE_HUD_Colour_Tertiary", C_TEXT_GREEN);
			updateHudColour(static_cast<HUDColourData*>(packet), HUDColour::ToggleState::TERTIARY_COLOUR);
			break;
		case NE_HUD_ColourType::NE_HUD_Colour_Reset:
			hkDrawText("NE_HUD_Colour_Reset", C_TEXT_GREEN);
			HUDColour::clear();
			break;
	}
}

void eventTrigger(NetworkEventBase* packet) {
	switch(packet->opcode) {
		case NE_EventType::NE_Event_BlueText:
			{
				hkDrawText("NE_HUD_Colour_Reset", C_TEXT_GREEN);
			auto message = static_cast<HUDMessageData*>(packet)->message;
			HUDMessage(message);
			}
			break;
		case NE_EventType::NE_Event_HAC_Sound:
			{
				hkDrawText("NE_HUD_Colour_Reset", C_TEXT_GREEN);
			auto index = static_cast<SoundEvent*>(packet)->index;
			PlayMPSound(EngineTypes::multiplayer_sound(index));
			}
		break;
		case NE_EventType::NE_Event_Sound:
			{
				hkDrawText("NE_HUD_Colour_Reset", C_TEXT_GREEN);
			auto index = static_cast<SoundEvent*>(packet)->index;
			PlayMPSound(EngineTypes::multiplayer_sound(index));
			}
		break;
	}
}

void _stdcall netEventHandler(NetworkEventBase* packet) {
	switch(packet->type) {
		case EventType::DATA_SYNC:
			hkDrawText("DATA_SYNC", C_TEXT_GREEN);
			dataSyncEvent(packet);
			break;
		case EventType::MEDALS:
			hkDrawText("MEDALS", C_TEXT_GREEN);
			medalEvent(packet);
			break;
		case EventType::HUD_CONTROL:
			hkDrawText("HUD_CONTROL", C_TEXT_GREEN);
			hudControl(packet);
			break;
		case EventType::EVENT:
			hkDrawText("EVENT", C_TEXT_GREEN);
			eventTrigger(packet);
			break;
		default:
			hkDrawText("UNKNOWN TYPE", C_TEXT_GREEN);
		}
}

void sendHandshake() {
	DSHandshake hs;
	hs.type = EventType::DATA_SYNC;
	hs.opcode = DataSyncOpcodes::DS_HANDSHAKE;
	hs.version.major = 2;
	hs.version.minor = 2;
	hs.version.patch = 1;
	hs.app = AppID::HAC2;
	sendPacket(&hs, sizeof(DSHandshake));
}

// Sehe did this ":)".
#define pushad __asm \
{ \
__asm push eax   \
__asm push ecx   \
__asm push edx   \
__asm push ebx   \
__asm push esi   \
__asm push edi   \
}

#define popad __asm \
{\
__asm pop edi   \
__asm pop esi   \
__asm pop ebx   \
__asm pop edx   \
__asm pop ecx   \
__asm pop eax   \
}

void __declspec(naked) netEventsStub() {
	__asm {
		mov eax, dword ptr ss : [esp + 0x0c]
		cmp eax, 5
		jne not_event
		pushad
		mov eax, dword ptr ss : [esp + 0x2c]
		push eax
		call netEventHandler
		//add esp, 4
		popad
		not_event:
		cmp eax, 4
		jmp BackToNetHandler
	}
}

void sendPacket(void* data, std::uint32_t len) {
	__asm {
		pushad
		call GetLocalPlayerIndex
		cmp eax, -1
		je not_in_server
		push eax
		mov edx, data
		mov eax, len
		mov NetworkEventPacket, eax
		mov eax, 5
		call ChatSend
		mov NetworkEventPacket, 0
		add esp, 4
		not_in_server:
		popad
	}
}

std::uint32_t buildChatPacket(wchar_t* chatPacket) {
	return NetworkEventPacket? NetworkEventPacket : wcslen(chatPacket);
}

} // NetEvents