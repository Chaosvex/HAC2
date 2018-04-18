#pragma once

#include <cstdint>

// blame Sehe for the awful design and naming here

namespace NetEvents {

enum EventType : std::uint8_t {
	ANTICHEAT, DATA_SYNC, HUD_CONTROL, EVENT, MEDALS
};

enum DataSyncOpcodes {
	DS_HANDSHAKE,
	DS_UNUSED,
	DS_GAMESPEED,
	DS_GRAVITY,
	DS_MTV,
	DS_SJ,
	DS_ZOMBIES,
	DS_TIMELEFT,
	DS_TIMEELAPSED
};

enum NE_EventType { 
	NE_Event_BlueText,
	NE_Event_Sound,
	NE_Event_HAC_Sound
};

enum NE_HUD_ColourType { 
	NE_HUD_Colour_Reset,
	NE_HUD_Colour_Reticle,
	NE_HUD_Colour_Primary,
	NE_HUD_Colour_Secondary,
	NE_HUD_Colour_Tertiary
};

enum MedalOpcodes { 
	M_KILL
};

#pragma pack(push, 1)

struct Version {
	std::uint8_t sehe_broke_something;
	std::uint8_t patch;
	std::uint8_t minor;
	std::uint8_t major;
};

enum AppID : std::uint32_t {
	NONE = 0,
	SAPP = 1,
	AC = 2,
	HAC2 = 4,
	REFLEX = 8, // :).
};

struct NetworkEventBase {
	EventType type;
	std::uint8_t opcode;
};

struct DSHandshake : public NetworkEventBase {
	AppID app;
	Version version;
};

template<typename T>
struct DSData : public NetworkEventBase {
	T data;
};

struct Medal_Kill : public NetworkEventBase {
	std::uint8_t killed;
	std::uint8_t killer;
	std::uint16_t flags;
	std::uint32_t metaID;
};

struct HUDMessageData : public NetworkEventBase {
	char message[64];
};

struct HUDColourData : public NetworkEventBase {
	std::uint8_t r, g, b;
};

struct SoundEvent : public NetworkEventBase {
	std::uint8_t index;
};

#pragma pack(pop)

} // NetEvents