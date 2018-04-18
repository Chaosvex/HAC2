#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>

namespace EngineTypes {

const std::uint32_t UNHI = 0x756E6869;
const std::uint32_t WPHI = 0x77706869;
const std::uint32_t GRHI = 0x67726869;
const std::uint32_t BITM = 0x6269746D;
const std::uint32_t HUDH = 0x68756423; //HUD#
const std::uint32_t DELA = 0x44654C61;

enum GAME_EVENTS : std::uint32_t {
	FALLING_DEATH = 1,
	GUARDIAN_KILL,
	VEHICLE_KILL,
	PLAYER_KILL,
	BETRAYED,
	SUICIDE,
	LOCAL_DOUBLE_KILL,
	LOCAL_KILLED_PLAYER,
	LOCAL_TRIPLE_KILL,
	LOCAL_KILLTACULAR,
	LOCAL_KILLING_SPREE,
	LOCAL_RUNNING_RIOT = 14,
	BETRAYED_PLAYER = 13,
	RACE_LAP_COMPLETE = 32,
	FLAG_CAPTURED = 33,
	UNKNOWN_CTF = 34, //won?
	UNKNOWN_CTF_2 = 35, //lost?
	PLAYER_FLAG_RETURN = 37,
	RACE_FASTEST_LAP = 38,
	ENEMY_RETURNED_FLAG = 40,
	ALLY_STOLE_FLAG = 41,
	ALLY_RETURNED_FLAG = 42,
	FRIENDLY_FLAG_IDLE_RETURNED = 43,
	ENEMY_FLAG_IDLE_RETURNED = 44
};

enum OVERLAY_STATES {
	NO_OVERLAY,
	BLANK,
	STARTING_NETWORK_SERVER,
	CONNECTING_TO_SERVER,
	NEGOTIATING_CONNECTION,
	CONNECTING_TO_SERVER_2,
	RETRYING_SERVER,
	CONNECTION_ESTABLISHED,
	LOADING_MAP,
	WAITING_FOR_NEW_GAME
};

enum CHAT_TYPE : std::uint32_t {
	CHAT_ALL,
	CHAT_TEAM,
	CHAT_VEHICLE,
	CHAT_EVENT = 5,
	HAC_HANDSHAKE = 0xFF
};

enum CAMERA_PERSPECTIVES : std::uint16_t {
	NONE,
	FIRST_PERSON = 2,
	DEAD = 3
};

#pragma pack(push, 1)

struct RenderedParticle {
	std::uint16_t index;
	std::uint16_t definition_index;
	std::uint8_t padding[4];
}; static_assert(sizeof(RenderedParticle) == 8, "RenderedParticle size incorrect!");

struct CameraControl {
	char isCameraEnabled;
	char isCameraDisabled;
	CAMERA_PERSPECTIVES perspective; // 0 none, 2 First Person, 3 Dead
	char animationStuff[48];
	WORD objectIndex;
	WORD objectID;
};

struct TagEntry {
	std::uint32_t class0;
	std::uint32_t class1;
	std::uint32_t class2;
	std::uint32_t tagID;
	char* name;
	std::uint32_t tagStruct;
	char Unknown[8];
};

struct TagTableHeader {
	TagEntry* TagTableEntryPointer;
	std::uint32_t FirstTagID;
	std::uint32_t MapID;
	std::uint32_t TagCount;
	std::uint32_t VerticesCount;
	std::uint32_t VerticesOffset;
	std::uint32_t IndicesCount;
	std::uint32_t IndicesOffset;
	std::uint32_t ModelDataSize;
	std::uint32_t Signature;
};

struct BSPChunkMeta {
	std::uint32_t offset;
	std::uint32_t size;
};

//Just made this up
struct SceneryTag {
	std::uint32_t sections; //BSP count?
	std::uint32_t address;
};

/*
 * Stores server information used for reconnecting after a map download.
 */
struct ConnectionDetails {
	std::uint32_t ip;
	BYTE unknown[14];
	USHORT port;
};

struct MapHeader {
	long Signature0; // head = "daeh"
	short Game; // Xbox = 5 / Trial = 6 / PC = 7 / CE = 0x261
	short Unknown0;
	long MapSize;
	char Unknown1[4];
	long IndexOffset;
	long MetaSize;
	char Unknown3[8];
	char MapName[14];
	char Unknown4[18];
	char Build[12]; // PC = 01.00.00.0564 / CE = 01.00.00.0609
	char Unknown5[20];
	char MapType; // SP = 0 / MP = 1 / UI = 2
	char Unknown6;
	short Unknown7;
	int checksum;
	char Unknown8[1940];
	long Signature1; // foot = "toof"
};

struct MapEntry {
	char* name;
	std::uint32_t checked;
	std::uint32_t forgot;
};

struct MapEntryCE : public MapEntry {
	std::uint32_t checksum;
};

enum multiplayer_sound {
	_multiplayer_sound_playball,
	_multiplayer_sound_game_over,
	_multiplayer_sound_one_min_to_win,
	_multiplayer_sound_30_secs_to_win,
	_multiplayer_sound_red_team_one_min_to_win,
	_multiplayer_sound_red_team_30_secs_to_win,
	_multiplayer_sound_blue_team_one_min_to_win,
	_multiplayer_sound_blue_team_30_secs_to_win,
	_multiplayer_sound_red_team_flag_taken,
	_multiplayer_sound_red_team_flag_returned,
	_multiplayer_sound_red_team_flag_captured,
	_multiplayer_sound_blue_team_flag_taken,
	_multiplayer_sound_blue_team_flag_returned,
	_multiplayer_sound_blue_team_flag_captured,
	_multiplayer_sound_double_kill,
	_multiplayer_sound_triple_kill,
	_multiplayer_sound_killtacular,
	_multiplayer_sound_running_riot,
	_multiplayer_sound_killing_spree,
	_multiplayer_sound_oddball,
	_multiplayer_sound_race,
	_multiplayer_sound_slayer,
	_multiplayer_sound_ctf,
	_multiplayer_sound_warthog,
	_multiplayer_sound_ghost,
	_multiplayer_sound_scorpion,
	_multiplayer_sound_countdown_timer,
	_multiplayer_sound_teleporter_activate,
	_multiplayer_sound_flag_failure,
	_multiplayer_sound_countdown_for_respawn,
	_multiplayer_sound_hill_move,
	_multiplayer_sound_player_respawn,
	_multiplayer_sound_koth,
	_multiplayer_sound_odd_ball,
	_multiplayer_sound_team_race,
	_multiplayer_sound_team_slayer,
	_multiplayer_sound_team_koth,
	_multiplayer_sound_blue_team_ctf,
	_multiplayer_sound_red_team_ctf,
	_multiplayer_sound_contest,
	_multiplayer_sound_control,
	_multiplayer_sound_hill_occupied,
	_multiplayer_sound_countdown_timer_end,
	_multiplayer_sound_ting,
	_multiplayer_sound,

	_multiplayer_sound_custom1 = _multiplayer_sound,
	_multiplayer_sound_custom2,
	_multiplayer_sound_custom3,
	_multiplayer_sound_custom4,
	_multiplayer_sound_custom5,
	_multiplayer_sound_custom6,
	_multiplayer_sound_custom7,
	_multiplayer_sound_custom8,
	_multiplayer_sound_custom9,
	_multiplayer_sound_custom10,
	_multiplayer_sound_custom11,
	_multiplayer_sound_custom12,
	_multiplayer_sound_custom13,
	_multiplayer_sound_custom14,
	_multiplayer_sound_custom15,
	_multiplayer_sound_custom16, // default max s_game_globals_multiplayer_information::sounds instances = 60

	_multiplayer_sound_custom,
};

struct ObjectEntry {
    WORD ObjectID;
    WORD Unknown0;
    WORD Unknown1;
    WORD size;
    DWORD offset;
};

struct ObjectsHeader {
	char Name[6]; // "objects"
	char Unknown0[26];
	std::uint16_t MaxObjs; // 0x800
	std::uint16_t ObjTableSize; // 0xC
	std::uint32_t Unknown1;
	std::uint32_t Signature; // data = "@t@d"
	std::uint16_t Objs;
	std::uint16_t CurMaxObjs;
	std::uint16_t CurObjs;
	std::uint16_t NextObjIndex;
	ObjectEntry* firstObject;
};

struct PlayerObject {
	long TagID;
	char Unknown0[88];
	float XCoord0;
	float YCoord0;
	float ZCoord0;
	float XVel;
	float YVel;
	float ZVel;
	float Pitch;
	float Yaw;
	float Roll;
	float XScale;
	float YScale;
	float ZScale;
	char Unknown1[44];
	char Team0; // Dead = Red = 0 / Blue = 1
	char Unknown2[31];
	float MaxHealth; // From coll tag.
	float MaxShields; // From coll tag.
	float Health;
	float Shields; // None = 0 / Full = 1 / Overshield = 3
	char Unknown3[28];
	short ShieldsRechargeTicks; // Time until shields recharge. / 1 sec = 30 ticks
	char Unknown4[18];
	long WeapObjID;
	long VehiObjID;
	short VehiSeat0;
	char Unknown5[6];
	float ShieldsTarget; // Something to do with Overshield draining?
	float FlashlightScale0; // Intensity of flashlight.
	long Unknown6;
	float FlashlightScale1; // Intensity of flashlight.
	float ShieldsHit; // Something to do with shields being hit?
	char Unknown7[76];
	char Team1; // Blue = Dead = 0 / Red = 1
	char Unknown8[123];
	char InvisMode; // No = 0x41 / Yes = 0x51
	char Unknown9;
	char FlashlightMode; // Off = 0 / On = 8
	char Unknown10;
	char CrouchButton; // Standing = 0 / Crouching = 1 / Jumping = 2
	char Unknown11[17];
	short PlayerID;
	long Unknown12;
	short BulletCount; // Increases with each shot?
	char Unknown13[14];
	float XAim0;
	float YAim0;
	float ZAim0;
	float XAim1;
	float YAim1;
	float ZAim1;
	float XAim2;
	float YAim2;
	float ZAim2;
	char Unknown14[12];
	float ForwardButton; // Negative means backward.
	float LeftButton; // Negative means right.
	char Unknown15[6];
	short Shooting0; // No = 0 / Yes = 0x3F80
	char Unknown16[24];
	char CrouchMode0; // Vehicle = 0 / Crouching = 3 / Standing = 4
	long Unknown17;
	char Shooting1; // No = 0 / Yes = 1
	char Unknown18;
	char CrouchMode1; // Also VehiSeat1. / Standing = 2 / Crouching = 3
	char Unknown19[74];
	char WeapType0; // Primary = 0 / Secondary = 1 / Ternary = 2 / Quartary = 3
	char Unknown20;
	char WeapType1; // Primary = 0 / Secondary = 1 / Ternary = 2 / Quartary = 3
	char Unknown21[3];
	long PrimaryWeapObjID;
	long secondaryweapObjID;
	long TernaryWeapObjID;
	long QuartaryWeapObjID;
	char Unknown22[20];
	char NadeType0; // Frag = 0 / Plasma = 1
	char Unknown23;
	char PrimaryNades;
	char SecondaryNades;
	char Unknown24[32];
	float FlashlightScale2; // Intensity of flashlight.
	float FlashlightEnergy; // Amount of flashlight energy left.
	char Unknown25[52];
	float InvisScale; // Visibility of invis.
	char Unknown26[244];
	char Shooting2; // No = 0 / Yes = 1
	char Unknown27[7];
	char WeapType2; // Primary = 0 / Secondary = 1 / Ternary = 2 / Quartary = 3
	char Unknown28;
	char NadeType1; // Frag = 0 / Plasma = 1
	char Unknown29[21];
	float XAim3;
	float YAim3;
	float ZAim3;
	float XAim4;
	float YAim4;
	float ZAim4;
	float XAim5;
	float YAim5;
	float ZAim5;
	char Unknown30[20];
	char Airborne; // No = 0 / Yes = 1
	char Unknown31[19];
	float XCoord1;
	float YCoord1;
	float ZCoord1;
	char Unknown32[32];
	float CrouchScale; // How crouched a player is.
};

struct Player {
	std::uint16_t playerID;            // Stats at 0x70EC
	std::uint16_t m_wPlayerID2;           // local player 0/1
	wchar_t name[12]; // Unicode / Max - 11 Chars + EOS (12 total)
	std::uint32_t _unknown;              // Always -1 / 0xFFFFFFFF
	std::uint32_t team;              // 0 = Red / 1 = Blue
	std::uint32_t m_dwSwapID;            // ObjectID
	std::uint16_t m_wSwapType;            // 8 = Vehicle / 6 = Weapon
	std::uint16_t m_wSwapSeat;            // Warthog - Driver = 0 / Passenger = 1 / Gunner = 2 / Weapon = -1
	std::uint32_t respawn_timer;
	std::uint32_t respawn_penalty; //adds time to respawn wait
	std::uint16_t objectIndex;
	std::uint16_t objectID;            // Matches against object table
	std::uint32_t m_dwUnknown3;          // Some sort of ID
	std::uint32_t m_dwSector;            // This is very, very interesting. BG is split into 25 location ID's. 1 -19
	std::uint32_t m_dwUnknown4;          // Always -1 / 0xFFFFFFFF
	std::uint32_t m_dwBulletCount;       // Something to do with bullets increases - weird.
	wchar_t m_wcPlayerName1[12]; // Unicode / Max - 11 Chars + EOS (12 total)
	std::uint32_t m_dwUnknown5;          // 02 00 FF FF
	std::uint32_t m_dwPlayerIndex;
	std::uint32_t effect_timer; //counts active camo down
	float m_fSpeedModifier;
	char unknown10[32];
	char unknows4[6];  // 144
	unsigned short killstreak;
	unsigned short combo;
	unsigned short killstreak_score;
	std::uint32_t kills;         // 156
	std::uint32_t unknown5;      // 160
	std::uint32_t assists;	   // 164
	std::uint32_t unknown11;    //168
	std::uint16_t suicides;      // 172
	std::uint16_t deaths;        // 174
	std::uint16_t notsuicides;
	char unknown7[22];  // 176
	long ctf_score;     //200
	char sfs[16]; // 204
	std::int32_t ping;          // 220
	std::uint32_t unknowns[2];
	std::uint32_t unknown_timer1;
	std::uint32_t unknown_timer2;
	float x, y, z;  // 248 position
	char unknown12[260]; // 260
}; static_assert(sizeof(Player) == 0x200, "Player size incorrect!");

struct PlayerServer {
	std::uint16_t playerID;            // Stats at 0x70EC
	std::uint16_t m_wPlayerID2;           // local player 0/1
	wchar_t name[12]; // Unicode / Max - 11 Chars + EOS (12 total)
	std::uint32_t _unknown;              // Always -1 / 0xFFFFFFFF
	std::uint32_t team;              // 0 = Red / 1 = Blue
	std::uint32_t m_dwSwapID;            // ObjectID
	std::uint16_t m_wSwapType;            // 8 = Vehicle / 6 = Weapon
	std::uint16_t m_wSwapSeat;            // Warthog - Driver = 0 / Passenger = 1 / Gunner = 2 / Weapon = -1
	std::uint32_t respawn_timer;
	std::uint32_t respawn_penalty; //adds time to respawn wait
	std::uint16_t objectIndex;
	std::uint16_t objectID;            // Matches against object table
	std::uint32_t m_dwUnknown3;          // Some sort of ID
	std::uint32_t m_dwSector;            // This is very, very interesting. BG is split into 25 location ID's. 1 -19
	std::uint32_t m_dwUnknown4;          // Always -1 / 0xFFFFFFFF
	std::uint32_t m_dwBulletCount;       // Something to do with bullets increases - weird.
	wchar_t m_wcPlayerName1[12]; // Unicode / Max - 11 Chars + EOS (12 total)
	std::uint32_t m_dwUnknown5;          // 02 00 FF FF
	std::uint32_t m_dwPlayerIndex;
	std::uint32_t effect_timer; //counts active camo down
	float m_fSpeedModifier;
	char unknown10[32];
	char unknows4[6];  // 144
	unsigned short killstreak;
	unsigned short combo;
	unsigned short killstreak_score;
	std::uint32_t kills;         // 156
	std::uint32_t unknown5;      // 160
	std::uint32_t assists;	   // 164
	std::uint32_t unknown11;    //168
	std::uint16_t suicides;      // 172
	std::uint16_t deaths;        // 174
	std::uint16_t notsuicides;
	char unknown7[22];  // 176
	long ctf_score;     //200
	char sfs[16]; // 204
	std::int32_t ping;          // 220
	std::uint16_t tk_number;     // 224
	char unknown8[2];
	std::uint32_t tk_timer;
	char unknown9[12];
	long player_dead;	// 244  -1 if players is dead
	float x, y, z;  // 248 position
	char unknown12[252]; // 260
}; static_assert(sizeof(PlayerServer) == 0x200, "Player server size incorrect!");

struct PlayerHeader {
	std::uint8_t m_bName[32];        // 'players'
	std::uint16_t m_wMaxSlots;        // Max number of slots/players possible
	std::uint16_t m_wSlotSize;        // Size of each Static_Player struct
	std::uint32_t m_dwUnknown;           // always 1?
	std::uint8_t w_bData[4];         // '@t@d' - translated as 'data'?
	std::uint16_t m_wIsInMainMenu;    // 0 = in game 1 = in main menu / not in game
	std::uint16_t m_wSlotsTaken;      // or # of players
	std::uint16_t m_wNextPlayerIndex; // Index # of the next player to join
	std::uint16_t m_wNextPlayerID;    // ID # of the next player to join
	Player* firstPlayer;   // Pointer to the first static player
};

struct Real2D;

struct AnchorOffset {
	std::int16_t x, y;
	operator Real2D() const;
};

struct Real2D {
	float x, y;
	operator AnchorOffset() const;
};

enum MapType : std::uint16_t {
	ALL,
	SOLO,
	MULTIPLAYER
};

template<typename Tag> struct Reflexive {
	std::uint32_t count;
	Tag* tag;
	std::uint32_t reserved;
};

enum ElemAnchor: std::uint16_t {
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
	CENTRE
};

struct LayoutElement {
	AnchorOffset offset;
	Real2D scale;
};

struct ColourInformation {
	std::uint8_t blue;
	std::uint8_t green;
	std::uint8_t red;
	std::uint8_t alpha;
};

struct ScalingFlags {
	std::uint16_t moop : 12;
	std::uint16_t use_high_res_scale : 1;
	std::uint16_t no_scale_size : 1;
	std::uint16_t no_scale_offset : 1;
};

struct AuxiliaryHUDMeter {
	char padding[20];
	LayoutElement bg_layout; //0x14
	ScalingFlags bg_scaling; //0x20
	char bg_padding2[22]; //0x22
	char bg_bitmap_padding[16]; //0x38
	ColourInformation bg_default_colour; //0x48
	ColourInformation bg_flash_colour; //0x4C
	float bg_flash_period; //0x50
	float bg_flash_delay; //0x54
	std::uint16_t bg_num_flashes; //0x58
	char bg_flash_flags; //0x5A
	char padding3; //0x5B
	float bg_flash_length; //0x5C
	ColourInformation bg_disabled_colour; //0x60
	char padding4[4]; //0x64
	std::uint16_t bg_sequence_index; //0x68
	char padding5[18]; //0x6A
	LayoutElement meter_layout; //0x7C
	ScalingFlags meter_scaling; //0x88
	char padding6[22]; //0x8A
	char meter_bitmap_padding[16]; //0xA0
	ColourInformation meter_min_colour; //0xB0
	ColourInformation meter_max_colour; //0xB4
	ColourInformation meter_flash_colour; //0xB8
	ColourInformation meter_empty_colour; //0xBC
	std::uint16_t meter_flags; //0xC0
	std::uint16_t meter_sequence_index; //0xC2
	char padding7[12]; //0xC4
	ColourInformation meter_disabled_colour; //0xD0
	char padding8[104]; //0xDC
}; //size - 324 bytes

struct AuxiliaryOverlay {
	LayoutElement layout;
	ScalingFlags scaling; //0x0C
	char padding[22]; //0x0E
	char bitmap_padding[16]; //0x24
	ColourInformation default_colour; //0x34
	ColourInformation flashing_colour; //0x38
	float flash_period; //0x3C
	float flash_delay; //0x40
	std::uint16_t num_flashes; //0x44
	char flash_flags; //0x46
	char padding2; //0x47
	float flash_length; //0x48
	ColourInformation disabled_colour; //0x4C
	char padding3[52]; //0x50
}; //size - 132 bytes

struct UnitHUD {
	ElemAnchor weapon_anchor;
	char padding[34]; //0x02
	LayoutElement hud_bg_layout;
	char hug_bg_scaling_flags_padding[2]; //0x30
	char padding2[22]; //0x32
	char hud_bg_bitmap_padding[16]; //0x48
	ColourInformation hud_bg_default_colour; //0x58
	ColourInformation hud_bg_flashing_colour; //0x5C
	float hud_bg_flash_period; //0x60
	float hud_bg_flash_delay; //0x64
	short hud_bg_num_flashes; //0x68
	char hud_bg_flash_flags_padding; //0x6A
	char padding3; //0x6B
	float hud_bg_flash_length; //0x6C
	ColourInformation hug_bg_disabled_colour; //0x70
	char padding4[24]; //0x74
	LayoutElement shield_panel_bg_layout; //0x8C
	char shield_panel_bg_scaling_flags_padding[2]; //0x98
	char padding5[22]; //0x9A
	char shield_panel_bg_bitmap_padding[16]; //0xB0
	ColourInformation shield_panel_bg_default_colour; //0xC0
	ColourInformation shield_panel_bg_flashing_colour; //0xC4
	float shield_panel_bg_flash_period; //0xC8
	float shield_panel_bg_flash_delay; //0xCC
	short shield_panel_bg_num_flashes; //0xD0
	char shield_panel_bg_flash_flags_padding; //0xD2
	char padding6; //0xD3
	float shield_panel_bg_flash_length; //0xD4
	ColourInformation shield_panel_bg_disabled_colour; //0xD8
	char padding7[24]; //0xDC
	LayoutElement shield_panel_meter_layout; //0xF4
	char shield_panel_meter_scaling_flags_padding[2]; //0x100
	char padding8[22]; //0x102
	char shield_panel_meter_bitmap_padding[16]; //0x118
	ColourInformation shield_panel_meter_min_colour; //0x128
	ColourInformation shield_panel_meter_max_colour; //0x12C
	ColourInformation shield_panel_meter_flash_colour; //0x130
	ColourInformation shield_panel_meter_empty_colour; //0x134
	char shield_panel_meter_flags_padding[2]; //0x138
	//char shield_panel_meter_min_value; //0x139
	std::uint16_t shield_panel_meter_sequence_index; //0x13A
	char padding9[12]; //0x13C
	ColourInformation shield_panel_meter_disabled_colour; //0x148
	char padding10[16]; //0x14C
	ColourInformation shield_panel_meter_overcharge_min_colour; //0x15C
	ColourInformation shield_panel_meter_overcharge_max_colour; //0x160
	ColourInformation shield_panel_meter_overcharge_flash_colour; //0x164
	ColourInformation shield_panel_meter_overcharge_empty_colour; //0x168
	char padding11[16]; //0x16C
	LayoutElement health_panel_bg_layout; //0x17C
	char health_panel_bg_scaling_flags_padding[2]; //0x188
	char padding12[22]; //0x18A
	char health_panel_bg_bitmap_padding[16]; //0x1A0
	ColourInformation health_panel_bg_default_colour; //0x1B0
	ColourInformation health_panel_bg_flashing_colour; //0x1B4
	float health_panel_bg_flash_period; //0x1B8
	float health_panel_bg_flash_delay; //0x1BC
	short health_panel_bg_num_flashes; //0x1C0
	char health_panel_bg_flash_flags_padding; //0x1C2
	char padding13; //0x1C3
	float health_panel_bg_flash_length; //0x1C4
	ColourInformation health_panel_bg_disabled_colour; //0x1C8
	char padding14[24]; //0x1CC
	LayoutElement health_panel_meter_layout; //0x1E4
	char health_panel_scaling_flags_padding[2]; //0x1F0
	char padding15[22]; //0x1F2
	char health_panel_bitmap_padding[16]; //0x208
	ColourInformation health_panel_meter_min_colour; //0x218
	ColourInformation health_panel_max_meter_colour; //0x21C
	ColourInformation health_panel_flash_meter_colour; //0x220
	ColourInformation health_panel_empty_meter_colour; //0x224
	char health_panel_meter_flags_padding[2]; //0x228
	char padding16[14]; //0x22A
	ColourInformation health_panel_meter_disabled_colour; //0x238
	char padding17[16]; //0x23C
	ColourInformation health_panel_meter_medium_health_left_colour; //0x24C
	char padding18[28]; //0x250
	LayoutElement motion_sensor_bg_layout; //0x26C
	char motion_sensor_bg_scaling_flags_padding[2]; //0x278
	char padding19[22]; //0x27A
	char motion_sensor_bg_bitmap_padding[16]; //0x290
	ColourInformation motion_sensor_bg_default_colour; //0x2A0
	ColourInformation motion_sensor_bg_flashing_colour; //0x2A4
	float motion_sensor_bg_flash_period; //0x2A8
	float motion_sensor_bg_flash_delay; //0x2AC
	short motion_sensor_bg_num_flashes; //0x2B0
	char motion_sensor_bg_flash_flags_padding; //0x2B2
	char padding20; //0x2B3
	float motion_sensor_bg_flash_length; //0x2B4
	ColourInformation motion_sensor_bg_disabled_colour; //0x2B8
	char padding21[24]; //0x2BC
	LayoutElement motion_sensor_foreground_layout; //0x2D4
	char motion_sensor_foreground_scaling_flags_padding[2]; //0x2E0
	char padding22[22]; //0x2E2
	char motion_sensor_foreground_bitmap_padding[16]; //0x2F8
	ColourInformation motion_sensor_foreground_default_colour; //0x308
	ColourInformation motion_sensor_foreground_flashing_colour; //0x30C
	float motion_sensor_foreground_flash_period; //0x310
	float motion_sensor_foreground_flash_delay; //0x314
	short motion_sensor_foreground_num_flashes; //0x318
	char motion_sensor_foreground_flash_flags_padding; //0x31A
	char padding23; //0x31B
	float motion_sensor_foreground_flash_length; //0x31C
	ColourInformation motion_sensor_foreground_disabled_colour; //0x320
	char padding24[56]; //0x324
	LayoutElement motion_sensor_centre_layout; //0x35C
	char padding25[60]; //0x368
	Reflexive<AuxiliaryOverlay> auxiliary_overlays; //0x3A4
	char padding27[16]; //0x3B0
	char padding26[12]; //0x3C0
	Reflexive<AuxiliaryHUDMeter> auxiliary_hud_meters; //0x3CC 
};


enum StateAttached : std::uint16_t {
	TOTAL_AMMO,
	LOADED_AMMO,
	HEAT,
	AGE,
	SECONDARY_WEAP_TOTAL_AMMO,
	SECONDARY_WEAP_LOADED, AMMO,
	DISTANCE_TO_TARGET,
	ELEVATION_TO_TARGET
};

struct StaticElement {
	StateAttached attached;
	char padding[2]; //0x02
	MapType map_type; //0x04
	char padding2[30]; //0x06
	LayoutElement layout;
	ScalingFlags scaling_flags; //0x30
	char padding4[22]; //0x32
	char interface_bitmap_padding[16]; //0x48
	ColourInformation default_colour; //0x58
	ColourInformation flashing_colour; //0x5C
	float flash_period; //0x60
	float flash_delay; //0x64
	std::uint16_t num_flashes; //0x68
	char flash_flags; //0x6A;
	char padding_alignment; //0x6B
	float flash_length; //0x6C
	ColourInformation disabled_colour; //0x70
	char padding7[4]; //0x74
	std::uint16_t sequence_index; //0x78
	char padding8[2]; //0x7A
	char multitexture_overlay_padding[12]; //0x7C
	char padding5[44]; //0x88
}; //size - 180 bytes

struct MeterElement {
	StateAttached attached;
	char padding[2]; //0x02
	MapType map_type; //0x04
	char padding2[30]; //0x06
	LayoutElement layout; //0x24
	char scaling_flags_padding[2]; //0x30
	char padding4[22]; //0x32
	char meter_bitmap_padding[16]; //0x48
	ColourInformation meter_maximum; //0x58
	ColourInformation meter_minimum; //0x5C
	ColourInformation flash_colour; //0x60
	ColourInformation empty_colour; //0x64
	char flags_padding[2]; //0x68
	//char minimum_meter_value; //0x69 - XML INFO IS WRONG 'ERE
	std::int16_t sequence_index;
	char alpha_multiplier;
	char alpha_bias;
	char value_scale;
	float opacity;
	float transulency;
	ColourInformation disabled_colour;
	char padding5[54]; //0x70
}; //size - 180 bytes

struct NumberElement {
	StateAttached attached;
	char padding[2]; //0x02
	MapType map_type; //0x04
	char padding2[30]; //0x06
	LayoutElement layout;
	char padding3[2]; //0x2E
	char scaling_flags_padding[2]; //0x30
	char padding4[22]; //0x32
	ColourInformation default_colour; //0x48
	ColourInformation flashing_colour; //0x4C
	float flash_period; //0x60
	float flash_delay; //0x64
	std::uint16_t num_flashes; //0x68
	char padding5[68]; //0x6A
}; //size - 160 bytes

enum ReticleTypes : std::uint16_t {
	AIM,
	ZOOM,
	CHARGE,
	RELOAD_WARNING,
	FLASH_HEAT,
	FLASH_TOTAL_AMMO,
	FLASH_BATTERY,
	RELOAD_OVERHEAT,
	FLASH_WHEN_FIRING_NO_AMMO,
	FLASH_WHEN_THROWING_NO_GRENADE,
	SHOULD_RELOAD_SECONDARY_TRIGGER,
	FLASH_SECONDARY_TOTAL_AMMO,
	FLASH_SECONDARY_RELOAD,
	FLASH_WHEN_FIRING_SECONDARY_TRIGGER_NO_AMMO,
	LOW_SECONDARY_AMMO_NOT_ENOUGH_TO_RELOAD,
	PRIMARY_TRIGGER_READY,
	SECONDARY_TRIGGER_READY,
	FLASH_WHEN_FIRING_DEPLETED_BATTERY
};

struct ReticleOverlay {
	LayoutElement layout;
	ScalingFlags scaling_flags; //0x0C
	char padding[22]; //0x0E
	ColourInformation default_colour; //0x24
	ColourInformation flashing_colour; //0x28
	float flash_period; //0x2C
	float flash_delay; //0x30
	std::uint16_t num_flashes; //0x34
	char padding2[54]; //0x36
}; //size - 108 bytes

struct Overlay {
	LayoutElement layout;
	ScalingFlags scaling_flags; //0x0C
	char padding[22]; //0x0E
	ColourInformation default_colour; //0x24
	ColourInformation flashing_colour; //0x28
	float flash_period; //0x2C
	float flash_delay; //0x30
	std::int16_t num_flashes; //0x34
	char flash_flags_padding; //0x36
	char padding2; //0x37
	float flash_length; //0x38
	ColourInformation disabled_colour; //0x3C
	char padding3[4]; //0x40
	std::uint16_t frame_rate; //0x44
	char padding4[2]; //0x46
	std::uint16_t sequence_index; //0x48
	std::uint32_t type_padding; //0x4A
	char padding5[58]; //0x4E
}; //size - 136 bytes

struct Reticle {
	ReticleTypes type;
	char padding[2]; //0x02
	MapType map_type; //0x04
	char padding2[30]; //0x06
	char reticle_bitmap[16]; //0x24
	Reflexive<ReticleOverlay> overlays; //0x34
	char padding3[40]; //0x40
}; //size - 104 bytes

struct OverlayElement {
	StateAttached attached;
	char padding[2]; //0x02
	MapType map_type; //0x04
	char padding2[30]; //0x06
	char overlay_bitmap_padding[16]; //0x24
	Reflexive<Overlay> overlays; //0x34
	char padding3[40]; //0x40
}; //static_assert(sizeof(OverlayElement) == 104, "OverlayElement size incorrect!"); //@todo VS2010 bug

struct WeaponHUD {
	char padding[16];
	char padding2[4]; //0x10
	std::uint16_t flash_cutoff_total_ammo; //0x14
	std::uint16_t flash_cutoff_loaded_ammo; //0x16
	std::uint16_t flash_cutoff_heat; //0x18
	std::uint16_t flash_cutoff_age; //0x1A
	char padding5[32]; //0x1C
	ElemAnchor weapon_hud_anchor; //0x3C
	char padding3[34]; //0x3E
	Reflexive<StaticElement> static_elements; //0x60
	Reflexive<MeterElement> meter_elements; //0x6C
	Reflexive<NumberElement> number_elements; //0x78
	Reflexive<Reticle> crosshairs; //0x84
	Reflexive<OverlayElement> overlay_elements; //0x90
	char padding4[12]; //0x9C
}; 

struct Bitmap {
	char padding[24];
	std::int32_t pixel_offset; //0x18
	std::int32_t pixel_count; //0x1C
	std::uint32_t bitmap; //0x20
	std::uint32_t base_address; //0x2C
}; //size - 48 bytes

struct BitmapMeta {
	char padding[96];
	Reflexive<Bitmap> bitmaps; //0x60
};

struct GrenadeHUD {
	ElemAnchor anchor;
	char padding[34]; //0x02
	LayoutElement bg; //0x24
	ScalingFlags bg_scaling_flags; //0x30
	char padding2[22]; //0x32
	char bg_bitmap_padding[16]; //0xB0
	ColourInformation bg_default_colour; //0x58
	ColourInformation bg_flashing_colour; //0x5C
	float bg_flash_period; //0x60
	float bg_flash_delay; //0x64
	std::uint16_t num_flashes; //0x68
	char padding_bg_flash_flags; //0x6A
	char padding3; //0x6B
	float bg_flash_length; //0x6C
	ColourInformation bg_disabled_colour; //0x70
	char padding4[4]; //0x74
	std::uint16_t bg_sequence_index; //0x78
	char padding5[2]; //0x7A
	char bg_multitex_overlay_padding[12];  //0x7C
	char padding6[4]; //0x88
	LayoutElement total_grenades_bg; //0x8C
	ScalingFlags total_grenades_bg_scaling_flags; //0x98
	char padding7[22]; //0x9A
	char total_grenades_bg_bitmap_padding[16]; //0xB0
	ColourInformation total_grenades_bg_default_colour; //0xC0
	ColourInformation total_grenades_bg_flashing_colour; //0xC4
	float total_grenades_bg_flash_period; //0xC8
	float total_grenades_bg_flash_delay; //0xCC
	std::uint16_t total_grenades_num_flashes; //0xD0
	char total_grenades_padding_bg_flash_flags; //0xD2
	char padding8; //0xD3
	float total_grenades_bg_flash_length; //0xD4
	ColourInformation total_grenades_bg_disabled_colour; //0xD8
	char padding9[4]; //0xDC
	std::uint16_t total_grenades_bg_sequence_index; //0xE0
	char padding10[18]; //0xE2
	LayoutElement total_grenade_numbers; //0xF4
	ScalingFlags total_grenade_numbers_scale; //0x100
	char padding15[22]; //0x102
	ColourInformation total_grenade_numbers_default_colour; //0x118
	ColourInformation total_grenade_numbers_flashing_colour; //0x11C
	float total_grenade_numbers__flash_period; //0x120
	float total_grenade_numbers__flash_delay; //0x124
	std::uint16_t total_grenade_numbers_flashes; //0x128
	char total_grenade_numbers_padding_flash_flags; //0x12A
	char padding11; //0x12B
	float total_grenade_numbers_flash_length; //0x12C
	ColourInformation total_grenade_numbers_disabled_colour; //0x130
	char padding12[4]; //0x134
	std::uint8_t total_grenades_max_num_digits; //0x138
	std::uint8_t total_grenades_numbers_flags; //0x139
	std::uint8_t total_grenades_num_fractional_digits; //0x13A
	char padding13[13]; //0x13B
	std::uint16_t total_grenades_num_flash_cutoff; //0x148
	char padding14[2]; //0x14A
	char total_grenades_overlay_bitmap_padding[16]; //0x14C
	Reflexive<Overlay> overlays; //0x15C
};

struct HUDCounter {
	char digitsBitmap[16];
	std::int8_t bitmapDigitWidth;
	std::int8_t screenDigitWidth;
	std::int8_t xOffset;
	std::int8_t yOffset;
	std::int8_t decimalPointWidth;
	std::int8_t colonWidth;
};

struct Resolution {
	std::uint32_t width;
	std::uint32_t height;
};

enum WIDGET_TYPES : std::uint16_t {
	CONTAINER, TEXT_BOX, SPINNER_LIST, COLUMN_LIST, GAME_MODEL,
	MOVIE, CUSTOM
};

enum CONTROLLER_INDEX : std::uint16_t {
	PLAYER_1, PLAYER_2, PLAYER_3, PLAYER_4, ANY_PLAYER
};

struct Dependency {
	std::uint32_t offset;
	std::uint32_t tagClass;
	std::uint32_t notsure[2];
}; static_assert(sizeof(Dependency) == 16, "Dependency size incorrect!");

struct WidgetFlags {
	std::uint32_t : 16;
	std::uint32_t FORCE_HANDLE_MOUSE : 1;
	std::uint32_t DO_NOT_PUSH_HISTORY : 1;
	std::uint32_t ALWAYS_USE_NIFTY_RENDER_EFFECTS : 1;
	std::uint32_t ALWAYS_USE_TAG_CONTROLLER_INDEX : 1;
	std::uint32_t RETURN_TO_MAIN_MENU_IF_NO_HISTORY : 1;
	std::uint32_t PASS_HANDLED_EVENTS_TO_ALL_CHILDREN : 1;
	std::uint32_t RENDER_REGARDLESS_OF_CONTROLLER_INDEX : 1;
	std::uint32_t PASS_UNHANDLED_EVENTS_TO_ALL_CHILDREN : 1;
	std::uint32_t DO_NOT_FOCUS_SPECIFIC_CHILD_WIDGET : 1;
	std::uint32_t D_PAD_LEFT_RIGHT_TABS_THROUGH_LIST_ITEMS : 1;
	std::uint32_t D_PAD_UP_DOWN_TABS_THROUGH_LIST_ITEMS : 1;
	std::uint32_t D_PAD_LEFT_RIGHT_TABS_THROUGH_CHILDREN : 1;
	std::uint32_t D_PAD_UP_DOWN_TABS_THROUGH_CHILDREN : 1;
	std::uint32_t FLASH_BACKGROUND_BITMAP : 1;
	std::uint32_t PAUSE_GAME_TIME : 1;
	std::uint32_t PASS_UNHANDLED_EVENTS_TO_FOCUSED_CHILD : 1;
}; static_assert(sizeof(WidgetFlags) == 4, "WidgetFlags size incorrect!");

enum DATA_INPUT_FUNCTION : std::uint16_t {
	_NONE,
	PLAYER_SETTINGS_MENU_UPDATE_DESC,
	UNUSED,
	PLAYLIST_SETTINGS_MENU_UPDATE_DESC,
	GAMETYPE_SELECT_MENU_UPDATE_DESC,
	MP_TYPE_MENU_UPDATE_DESC,
	SOLO_LEVEL_SELECT_UPDATE,
	DIFFICULTY_MENU_UPDATE_DESC,
	BUILD_NUMBER,
	SERVER_LIST_UPDATE,
	NETWORK_PREGAME_STATUS_UPDATE,
	SPLITSCREEN_PREGAME_STATUS_UPDATE,
	NETWORK_SPLITSCREEN_PREJOIN_PLAYERS,
	MP_PROFILE_LIST_UPDATE,
	THREE_WIDE_PLAYER_PROFILE_LIST_UPDATE,
	PLAYER_PROFILE_EDIT_SELECT_MENU_UPDATE,
	PLAYER_PROFILE_SMALL_MENU_UPDATE,
	GAME_SETTINGS_LISTS_TEXT_UPDATE,
	SOLO_GAME_OBJECTIVE_TEXT,
	COLOUR_PICKER_UPDATE,
	GAME_SETTINGS_LISTS_PICTURE_UPDATE,
	MAIN_MENU_FAKE_ANIMATE,
	MP_LEVEL_SELECT_UPDATE,
	GET_ACTIVE_PLAYER_PROFILE_NAME,
	GET_EDIT_PLAYER_PROFILE_NAME,
	GET_EDIT_GAME_SETTINGS_NAME,
	GET_ACTIVE_PLAYER_PROFILE_COLOUR,
	MP_SET_TEXTBOX_MAP_NAME,
	MP_SET_TEXTBOX_GAME_RULESET,
	MP_SET_TEXTBOX_TEAMS_NO_TEAMS,
	MP_SET_TEXTBOX_SCORE_LIMIT,
	MP_SET_TEXTBOX_SCORE_LIMIT_TYPE,
	MP_SET_BITMAP_FOR_MAP,
	MP_SET_BITMAP_FOR_RULESET,
	MP_SET_TEXTBOX_NUMBER_OF_PLAYERS,
	MP_EDIT_PROFILE_SET_RULE_TEXT,
	SYSTEM_LINK_STATUS_CHECK,
	MP_GAME_DIRECTIONS,
	TEAMS_NO_TEAMS_BITMAP_UPDATE,
	WARN_IF_DIFFERENT_WILL_NUKE_SAVED_GAME,
	DIM_IF_NO_NETWORK_CABLE,
	PAUSE_GAME_SET_TEXTBOX_INVERTED,
	DIM_UNLESS_TWO_CONTROLLERS,
	CONTROLS_UPDATE_MENU,
	VIDEO_MENU_UPDATE,
	GAMESPY_SCREEN_UPDATE,
	COMMON_BUTTON_BAR_UPDATE,
	GAMEPAD_UPDATE_MENU,
	SERVER_SETTINGS_UPDATE,
	AUDIO_MENU_UPDATE,
	MP_PROFILE_VEHICLES_UPDATE,
	SOLO_MAP_LIST_UPDATE,
	MP_MAP_LIST_UPDATE,
	GAMETYPE_SELECT_LIST_UPDATE,
	GAMETYPE_EDIT_LIST_UPDATE,
	LOAD_GAME_LIST_UPDATE,
	CHECKING_FOR_UPDATES,
	DIRECT_IP_CONNECT_UPDATE,
	NETWORK_SETTINGS_UPDATE,

	//HAC callbacks
	OPTIC_MENU_UPDATE,
	CHAT_MENU_UPDATE,
	DATA_INPUT_FUNCTIONS_END
}; //static_assert(sizeof(DATA_INPUT_FUNCTION) == 2, "DATA_INPUT_FUNCTION size incorrect!"); //@todo VS2010 bug

struct GameDataInputFunctions {
	DATA_INPUT_FUNCTION function;
	std::uint8_t padding[34];
}; //static_assert(sizeof(GameDataInputFunctions) == 36, "GameDataInputFunctions size incorrect!");

struct EventHandlerFlags {
	std::uint32_t : 20;
	std::uint32_t TRY_BRANCH_ON_FAILURE : 1;
	std::uint32_t : 1;
	std::uint32_t GO_BACK_TO_PREVIOUS_WIDGET : 1;
	std::uint32_t REPLACE_SELF_WITH_WIDGET : 1;
	std::uint32_t RUN_FUNCTION : 1;
	std::uint32_t GIVE_FOCUS_TO_WIDGET : 1;
	std::uint32_t RELOAD_OTHER_WIDGET : 1;
	std::uint32_t RELOAD_SELF : 1;
	std::uint32_t OPEN_WIDGET : 1;
	std::uint32_t CLOSE_ALL_WIDGETS : 1;
	std::uint32_t CLOSE_OTHER_WIDGET : 1;
	std::uint32_t CLOSE_CURRENT_WIDGET : 1;
}; static_assert(sizeof(EventHandlerFlags) == 4, "EventHandlerFlags size incorrect!");

enum EVENT_TYPES : std::uint16_t {
	A_BUTTON,
	B_BUTTON,
	X_BUTTON,
	Y_BUTTON,
	BLACK_BUTTON,
	WHITE_BUTTON,
	LEFT_TRIGGER,
	RIGHT_TRIGGER,
	D_PAD_UP,
	D_PAD_DOWN,
	D_PAD_LEFT,
	D_PAD_RIGHT,
	START_BUTTON,
	BACK_BUTTON,
	LEFT_THUMB,
	RIGHT_THUMB,
	LEFT_ANALOGUE_STICK_UP,
	LEFT_ANALOGUE_STICK_DOWN,
	LEFT_ANALOGUE_STICK_LEFT,
	LEFT_ANALOGUE_STICK_RIGHT,
	RIGHT_ANALOGUE_STICK_UP,
	RIGHT_ANALOGUE_STICK_DOWN,
	RIGHT_ANALOGUE_STICK_LEFT,
	RIGHT_ANALOGUE_STICK_RIGHT,
	CREATED,
	DELETED,
	GET_FOCUS,
	LOSE_FOCUS,
	LEFT_MOUSE,
	MIDDLE_MOUSE,
	RIGHT_MOUSE,
	_DOUBLE_CLICK,
	CUSTOM_ACTIVATION,
	POST_RENDER
};

enum EVENT_HANDLER_FUNCTIONS : std::uint16_t {
	___NONE,
	LIST_GOTO_NEXT_ITEM,
	LIST_GOTO_PREVIOUS_ITEM,
	EMPTY_1, EMPTY_2,
	INIT_SP_LEVEL_LIST_SOLO,
	INIT_SP_LEVEL_LIST_COOP,
	DISPOSE_SP_LEVEL_LIST,
	SOLO_LEVEL_SET_MAP,
	SET_DIFFICULTY,
	START_NEW_GAME,
	PAUSE_GAME_RESTART_AT_CHECKPOINT,
	PAUSE_GAME_RESTART_LEVEL,
	PAUSE_GAME_RETURN_TO_MAIN_MENU,
	CLEAR_MP_PLAYER_JOINS,
	JOIN_CONTROLLER_TO_MP_GAME,
	INIT_NETWORK_GAME_SERVER_LIST,
	START_NETWORK_GAME_SERVER,
	DISPOSE_NETWORK_GAME_SERVER_LIST,
	SHUTDOWN_NETWORK_GAME,
	NETWORK_GAME_JOIN_FROM_SERVER_LIST,
	SPLIT_GAME_SCREEN_INIT,
	COOP_GAME_INIT,
	MAIN_MENU_INIT,
	MP_TYPE_MENU_UNIT,
	PICK_PLAY_STAGE_FOR_QUICK_START,
	MP_LEVEL_LIST_INIT,
	MP_LEVEL_LIST_DISPOSE,
	MP_LEVEL_SELECT,
	MP_PROFILES_LIST_INIT,
	MP_PROFILES_LIST_DISPOSE,
	MP_PROFILES_SET_FOR_GAME,
	SWAP_PLAYER_TEAM,
	NETWORK_GAME_JOIN_PLAYER,
	PLAYER_PROFILE_LIST_INIT,
	PLAYER_PROFILE_LIST_DISPOSE,
	THREE_WIDE_PLAYER_PROFILE_SET_FOR_GAME,
	ONE_WIDE_PLAYER_PROFILE_SET_FOR_GAME,
	MP_PROFILE_BEGIN_EDITING,
	MP_PROFILE_END_EDITING,
	MP_PROFILE_SET_GAME_ENGINE,
	MP_PROFILE_CHANGE_NAME,
	MP_PROFILE_SET_CTF_RULES,
	MP_PROFILE_SET_KOTH_RULES,
	MP_PROFILE_SET_SLAYER_RULES,
	MP_PROFILE_SET_ODDBALL_RULES,
	MP_PROFILE_SET_RACING_RULES,
	MP_PROFILE_SET_PLAYER_OPTIONS,
	MP_PROFILE_SET_ITEM_OPTIONS,
	MP_PROFILE_SET_INIDICATOR_OPTIONS,
	MP_PROFILE_INIT_GAME_ENGINE,
	MP_PROFILE_INIT_NAME,
	MP_PROFILE_INIT_CTF_RULES,
	MP_PROFILE_INIT_KOTH_RULES,
	MP_PROFILE_INIT_SLAYER_RULES,
	MP_PROFILE_INIT_ODDBALL_RULES,
	MP_PROFILE_INIT_RACING_RULES,
	MP_PROFILE_INIT_PLAYER_OPTIONS,
	MP_PROFILE_INIT_ITEM_OPTIONS,
	MP_PROFILE_INIT_INDICATOR_OPTIONS,
	MP_PROFILE_SAVE_CHANGES,
	COLOUR_PICKER_MENU_INIT,
	COLOUR_PICKER_MENU_DISPOSE,
	COLOUR_PICKER_SELECT_COLOUR,
	PLAYER_PROFILE_BEGIN_EDITING,
	PLAYER_PROFILE_END_EDITING,
	PLAYER_PROFILE_CHANGE_NAME,
	PLAYER_PROFILE_SAVE_CHANGES,
	PLAYER_PROFILE_INIT_CONTROL_SETTINGS,
	PLAYER_PROFILE_INIT_ADVANCED_CONTROL_SETTINGS,
	PLAYER_PROFILE_SAVE_CONTROL_SETTINGS,
	PLAYER_PROFILE_SAVE_ADVANCED_CONTROL_SETTINGS,
	MP_GAME_PLAYER_QUIT,
	MAIN_MENU_SWITCH_TO_SOLO_GAME,
	REQUEST_DELETE_PLAYER_PROFILE,
	REQUEST_DELETE_PLAYLIST_PROFILE,
	FINAL_DELETE_PLAYER_PROFILE,
	FINAL_DELETE_PLAYLIST_PROFILE,
	CANCEL_PROFILE_DELETE,
	CREATE_AND_EDIT_PLAYLIST_PROFILE,
	CREATE_AND_EDIT_PLAYER_PROFILE,
	NETWORK_GAME_SPEED_START,
	NETWORK_GAME_DELAY_START,
	NETWORK_SERVER_ACCEPT_CONNECTION,
	NETWORK_SERVER_DEFER_START,
	NETWORK_SERVER_ALLOW_START,
	DISABLE_IF_NO_XDEMOS,
	RUN_XDEMOS,
	SINGLE_PLAYER_RESET_CONTROLLER_CHOICES,
	SINGLE_PLAYER_SET_PLAYER1_CONTROLLER_CHOICE,
	SINGLE_PLAYER_SET_PLAYER2_CONTROLLER_CHOICE,
	ERROR_IF_NO_NETWORK_CONNECTION,
	START_SERVER_IF_NONE_ADVERTISED,
	NETWORK_GAME_UNJOIN_PLAYER,
	CLOSE_IF_NOT_EDITING_PLAYER,
	EXIT_TO_XBOX_DASHBOARD,
	NEW_CAMPAIGN_CHOSEN,
	NEW_CAMPAIGN_DECISION,
	POP_HISTORY_STACK_ONCE,
	DIFFICULTY_MENU_INIT,
	BEGIN_MUSIC_FADE_OUT,
	NEW_GAME_IF_NO_PLAYER_PROFILES,
	EXIT_GRACEFULLY_TO_XBOX_DASHBOARD,
	PAUSE_GAME_INVERT_PITCH,
	START_NEW_COOP_GAME,
	PAUSE_GAME_INVERT_SPINNER_GET,
	PAUSE_GAME_INVERT_SPINNER_SET,
	MAIN_MENU_QUIT_GAME,
	MOUSE_EMIT_ACCEPT_EVENT,
	MOUSE_EMIT_BACK_EVENT,
	MOUSE_EMIT_DPAD_LEFT_EVENT,
	MOUSE_EMIT_DPAD_RIGHT_EVENT,
	MOUSE_SPINNER_THREE_WIDE_CLICK,
	CONTROLS_SCREEN_INIT,
	VIDEO_SCREEN_INIT,
	CONTROLS_BEGIN_BINDING,
	GAMESPY_SCREEN_INIT,
	GAMESPY_SCREEN_DISPOSE,
	GAMESPY_SELECT_HEADER,
	GAMESPY_SELECT_ITEM,
	GAMESPY_SELECT_BUTTON,
	PLAYER_PROFILE_INIT_MOUSE_SETTINGS,
	PLAYER_PROFILE_CHANGE_MOUSE_SETTINGS,
	PLAYER_PROFILE_INIT_AUDIO_SETTINGS,
	PLAYER_PROFILE_CHANGE_AUDIO_SETTINGS,
	PLAYER_PROFILE_CHANGE_VIDEO_SETTINGS,
	CONTROLS_SCREEN_DISPOSE,
	CONTROLS_SCREEN_CHANGE_SETTINGS,
	MOUSE_EMIT_X_EVENT,
	GAMEPAD_SCREEN_INIT,
	GAMEPAD_SCREEN_DISPOSE,
	GAMEPAD_SCREEN_CHANGE_GAMEPADS,
	GAMEPAD_SCREEN_SELECT_ITEM,
	MOUSE_SCREEN_DEFAULTS,
	AUDIO_SCREEN_DEFAULTS,
	VIDEO_SCREEN_DEFAULTS,
	CONTROLS_SCREEN_DEFAULTS,
	PROFILE_SET_EDIT_BEGIN,
	PROFILE_MANAGER_DELETE,
	PROFILE_MANAGER_SELECT,
	GAMESPY_DISMISS_ERROR,
	SERVER_SETTINGS_INIT,
	SERVER_SETTINGS_EDIT_SERVER_NAME,
	SERVER_SETTINGS_EDIT_SERVER_PASSWORD,
	SERVER_SETTINGS_START_GAME,
	VIDEO_TEST_DIALOGUE_INIT,
	VIDEO_TEST_DIALOGUE_DISPOSE,
	VIDEO_TEST_DIALOGUE_ACCEPT,
	GAMESPY_DISMISS_FILTERS,
	GAMESPY_UPDATE_FILTER_SETTINGS,
	GAMESPY_BACK_HANDLER,
	MOUSE_SPINNER_ONE_WIDE_CLICK,
	CONTROLS_BACK_HANDLER,
	CONTROLS_ADVANCED_LAUNCH,
	CONTROLS_ADVANCED_OK,
	MP_PAUSE_MENU_OPEN,
	MP_GAME_OPTIONS_OPEN,
	MP_CHOOSE_TEAM,
	MP_PROFILE_INIT_VEHICLE_OPTIONS,
	MP_PROFILE_SAVE_VEHICLE_OPTIONS,
	SINGLE_PREVIOUS_CL_ITEM_ACTIVIATED,
	MP_PROFILE_INIT_TEAMPLAY_OPTIONS,
	MP_PROFILE_SAVE_TEAMPLAY_OPTIONS,
	MP_GAME_OPTIONS_CHOOSE,
	EMIT_CUSTOM_ACTIVATION_EVENT,
	PLAYER_PROFILE_CANCEL_AUDIO_SET,
	PLAYER_PROFILE_INIT_NETWORK_OPTIONS,
	PLAYER_PROFILE_SAVE_NETWORK_OPTIONS,
	CREDITS_POST_RENDER,
	DIFFICULTY_ITEM_SELECT,
	CREDITS_INIT,
	CREDITS_DISPOSE,
	GAMESPY_GET_PATCH,
	VIDEO_SCREEN_DISPOSE,
	CAMPAIGN_MENU_INIT,
	CAMPAIGN_MENU_CONTINUE,
	LOAD_GAME_MENU_INIT,
	LOAD_GAME_MENU_DISPOSE,
	LOAD_GAME_MENU_ACTIVATED,
	SOLO_MENU_SAVE_CHECKPOINT,
	MP_TYPE_SET_MODE,
	CHECKING_FOR_UPDATES_OK,
	CHECKING_FOR_UPDATES_DISMISS,
	DIRECT_IP_CONNECTION_INIT,
	DIRECT_IP_CONNECTION_GO,
	DIRECT_IP_EDIT_FIELD,
	NETWORK_SETTINGS_EDIT_PORT,
	NETWORK_SETTINGS_DEFAULTS,
	LOAD_GAME_MENU_DELETE_REQUEST,
	LOAD_GAME_MENU_DELETE_FINISH,

	//HAC callbacks
	CHAT_SCREEN_INIT,
	CHAT_SCREEN_DEFAULTS,
	CHAT_SCREEN_SET,
	OPTIC_SCREEN_INIT,
	OPTIC_SCREEN_DEFAULTS,
	OPTIC_SCREEN_SET,
	HAC_CONTROLS_SCREEN_INIT,
	HAC_CONTROLS_SCREEN_DEFAULTS,
	HAC_CONTROLS_SET,
	GENERAL_SCREEN_INIT,
	GENERAL_SCREEN_DEFAULTS,
	GENERAL_SCREEN_SET,
	HAC_OPTIONS_BEGIN_EDITING,
	HAC_OPTIONS_END_EDITING,
	HAC_OPTIONS_SAVE_PREFERENCES,
	UI_EVENT_CALLBACKS_END
};

struct EventHandler {
	EventHandlerFlags flags;
	EVENT_TYPES eventType;
	EVENT_HANDLER_FUNCTIONS function;
	Dependency widgetTag;
	Dependency soundEffect;
	char script[32];
}; //static_assert(sizeof(EventHandlers) == 72, "EventHandlers size incorrect!"); //@todo VS2010 bug

enum REPLACE_FUNCTION : std::uint16_t {
	__NONE,
	WIDGET_CONTROLLER,
	_BUILD_NUMBER,
	PROCESS_ID
};

struct SearchReplaceFunction {
	char searchString[32];
	REPLACE_FUNCTION function;
}; //static_assert(sizeof(SearchReplaceFunction) == 34, "SearchReplaceFunction size incorrect!"); //@todo VS2010 bug

enum ELEMENT_JUSTIFICATION : std::uint16_t {
	LEFT_JUSTIFY, RIGHT_JUSTIFY, CENTRE_JUSTIFY
};

struct TextBoxFlags {
	std::uint16_t : 12;
	std::uint16_t DO_NOT_DO_THAT_WEIRD_FOCUS_TEST : 1;
	std::uint16_t FLASHING : 1;
	std::uint16_t PASSWORD : 1;
	std::uint16_t EDITABLE : 1;
}; static_assert(sizeof(TextBoxFlags) == 2, "TextBoxFlags size incorrect!");

struct ListItemFlags {
	std::uint16_t : 12;
	std::uint16_t LIST_SINGLE_PREVIEW_NO_SCROLL : 1;
	std::uint16_t LIST_ITEMS_ONLY_ONE_TOOLTIP : 1;
	std::uint16_t LIST_ITEMS_FROM_STRING_LIST_TAG : 1;
	std::uint16_t LIST_ITEMS_GENERATED_IN_CODE : 1;
}; static_assert(sizeof(ListItemFlags) == 2, "ListItemFlags size incorrect!");

struct ConditionalWidgetFlags {
	std::uint8_t : 7;
	std::uint8_t LOAD_IF_EVENT_HANDLER_FAILS : 1;
}; static_assert(sizeof(ConditionalWidgetFlags) == 1, "ConditionalWidgetFlags size incorrect!");

struct ConditionalWidget {
	Dependency widgetTag;
	char name[32];
	ConditionalWidgetFlags flags;
	std::uint8_t padding[3];
	std::uint16_t customControllerIndex;
	std::uint16_t verticalOffset;
	std::uint16_t horizontalOffset;
	std::uint8_t padding2[22];
}; static_assert(sizeof(ConditionalWidget) == 80, "ConditionalWidget size incorrect!");

struct ChildWidgetFlags {
	std::uint8_t : 7;
	std::uint8_t USE_CUSTOM_CONTROLLER_INDEX : 1;
}; static_assert(sizeof(ChildWidgetFlags) == 1, "ChildWidgetFlags size incorrect!");

struct ChildWidget {
	Dependency widgetTag;
	char name[32];
	ChildWidgetFlags flags;
	std::uint8_t padding[3];
	std::uint16_t customControllerIndex;
	std::uint16_t verticalOffset;
	std::uint16_t horizontalOffset;
	std::uint8_t padding2[22];
}; static_assert(sizeof(ChildWidget) == 80, "ChildWidget size incorrect!");

struct UIWidget {
	WIDGET_TYPES type;
	CONTROLLER_INDEX controllerIndex;
	char name[32];
	std::int16_t boundTop, boundLeft, boundBottom, boundRight;
	WidgetFlags flags;
	std::int32_t autoCloseTimer, autoCloseFadeTime;
	Dependency bgBitmap;
	Reflexive<GameDataInputFunctions> functions;
	Reflexive<EventHandler> eventHandlers;
	Reflexive<SearchReplaceFunction> replaceFunctions;
	std::uint8_t padding4[128];
	Dependency textBoxUnicodeStringsList;
	Dependency textBoxTextFont;
	ColourInformation textBoxTextColour;
	std::uint8_t padding5[12];
	ELEMENT_JUSTIFICATION textBoxJustify;
	TextBoxFlags textBoxFlags;
	std::uint8_t padding6[14];
	std::int16_t textBoxStringListIndex;
	std::int16_t textBoxHorizontalOffset;
	std::int16_t textBoxVerticalOffset;
	std::uint8_t padding7[28];
	ListItemFlags listItemFlags;
	std::uint8_t padding8[2];
	Dependency spinnerListHeaderBitmap;
	Dependency spinnerListFooterBitmap;
	std::int16_t spinnerListHeaderBoundsTop;
	std::int16_t spinnerListHeaderBoundsLeft;
	std::int16_t spinnerListHeaderBoundsBottom;
	std::int16_t spinnerListHeaderBoundsRight;
	std::int16_t spinnerListFooterBoundsTop;
	std::int16_t spinnerListFooterBoundsLeft;
	std::int16_t spinnerListFooterBoundsBottom;
	std::int16_t spinnerListFooterBoundsRight;
	std::uint8_t padding9[32];
	Dependency columnListExtendedDescWidget;
	std::uint8_t padding10[288];
	Reflexive<ConditionalWidget> conditionalWidgets;
	std::uint8_t padding11[256];
	Reflexive<ChildWidget> childWidgets;
};

//Only reversed enough to get things working
struct UIWidgetGroupMeta {
	std::uint16_t size; //always 112
	std::uint16_t maxWidgets; //always 32768
	std::uint16_t index;
	std::uint16_t unknown;
}; static_assert(sizeof(UIWidgetGroupMeta) == 8, "UIWidgetGroupMeta size incorrect!");

struct ResolutionSpinnerEntry {
	std::uint32_t width;
	std::uint32_t height;
	char label[32];
	std::uint32_t numRefreshRates; //numbers of entries in the below array
	std::uint32_t refreshRates[8]; //valid refresh rates for this resolution
}; static_assert(sizeof(ResolutionSpinnerEntry) == 76, "ResolutionSpinnerEntry size incorrect!");

struct UIWidgetGroup {
	std::uint32_t tagID;
	char* tagName; //the entire tag is here
	std::int16_t customControllerIndex; //possibly
	std::int16_t horizontalOffset;
	std::int16_t verticalOffset;
	std::int16_t index; //at least possibly
	float didNotCheck;
	std::int16_t unknown;
	std::uint8_t padding[14]; //didn't attempt to reverse
	float alpha; //affects all widgets in the 'group'
	UIWidgetGroup* previous;
	UIWidgetGroup* next;
	UIWidgetGroup* parent;
	UIWidgetGroup* child;
	UIWidgetGroup* child2; //not really sure what this is about - 56
	std::uint8_t padding2[4];
	std::uint16_t stringIndex;
	std::uint16_t unknown2; //something to do with scrolling
	void* customFunctionData; //maybe - related to 'list items from function'
	std::uint16_t customFunctionDataEntries; //number of entries in above array
	std::uint16_t unknown3;
	std::uint32_t padding4;
	UIWidgetGroup* extendedDescription;
	std::uint8_t padding5[12];
	UIWidgetGroupMeta meta;
	UIWidgetGroupMeta* prevMeta;
	UIWidgetGroupMeta* nextMeta;
}; static_assert(sizeof(UIWidgetGroup) == 112, "UIWidgetGroup size incorrect!");

enum UICallbackAction : std::uint32_t { // not sure about this
	CANCEL_ACTION,
	CONTINUE_ACTION
};

}

#pragma pack(pop)