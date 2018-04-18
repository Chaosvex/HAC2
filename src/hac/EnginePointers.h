#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <vector>
#include "Shared.h"

namespace EngineTypes {
	struct CameraControl;
	struct TagTableHeader;
	struct PlayerHeader;
	struct MapEntry;
	struct Resolution;
	struct ObjectsHeader;
}

/*
 * Function declarations
 */
void ConsoleFadeFixCC();
void LoadFix();
void loadChecksums();
void AddChecksumCC();
void serverJoinCave();
void ConsoleReadCC();
void WriteCacheCC();
void TestCC();
void SavePasswordStub();
void mapLoadStub();
void SoundEventStub();
void SearchPathSwitchCC();
void onGameLaunch();
void PasswordMaskerStub();
void FoVRedirect();
void postMapLoad();
void HUDCounterScale();
void resolutionNotify();
void PlayerEventStub();
void SpreeHandlerCodecave();
void chatHandlerStub();
std::uint32_t overlayTrigger();

/*
 * Codecave pointers
 */
extern std::uintptr_t ContinueFoV;
extern std::uintptr_t BackToJoining;
extern std::uintptr_t BackToLoading;
extern std::uintptr_t DoChecksum;
extern std::uintptr_t ContinueChecksum;
extern std::uintptr_t ProcessCommands;
extern std::uintptr_t BackToHalo;
extern std::uintptr_t BackToPathLoad;
extern std::uintptr_t LoadMap;
//extern std::uintptr_t ContinueChat;
extern std::uintptr_t ContinueMotdLoad;
extern std::uintptr_t ContinueCounterRender;
extern std::uintptr_t ContinuePlayerEvent;

/*
 * Function pointers
 */
extern std::uintptr_t ChatSend;
extern std::uintptr_t ConsolePrint;
extern std::uintptr_t GenMD5;
extern std::uintptr_t HUDPrint;
extern std::uintptr_t SoundPlay;
extern std::uintptr_t Connect;
extern std::uintptr_t CommandExecute;
extern std::uintptr_t DisplayMenu;
extern std::uintptr_t ChatLocalSend;
extern std::uintptr_t GetLocalPlayerIndex;

/*
 * Variable pointers
 */
extern BYTE* pKeyboardBuffer;
extern BYTE* pMouseBuffer;
extern float* pDrawDistance;
extern char* productType;
extern char* serverAddress;
extern short* histBuffLen;
extern char* histBuff;
extern char* pCurrentMap;
extern wchar_t* serverName;
extern char* VehicleControl;
extern EngineTypes::CameraControl* pCameraControl;
extern EngineTypes::TagTableHeader* pTagTableHeader;
extern EngineTypes::PlayerHeader** pPlayerHeader;
extern EngineTypes::MapEntry** pMapTable;
extern DWORD* overlayControl;
extern char* pProfilePath;
extern std::int16_t* stringIndex; //'Attention' box string index
extern bool* displayAttentionBox;
extern std::uint16_t* pGametypeIconBGXPos;
extern float* pGametypeIconBGWidth;
extern std::vector<std::uint16_t*> pGametypeIconOffsets;
extern std::vector<float*> pRadarMask;
extern EngineTypes::Resolution* pResolution;
extern EngineTypes::ObjectsHeader** pObjectsHeader;
extern std::uint8_t* activeCamoDisable;
extern std::uint8_t* masterVolume;
typedef std::uint32_t (__thiscall *oOverlayResolve)();
extern oOverlayResolve originalOverlayResolve;

/*
 * Immediate operand pointers
 */
extern uint32_t versionNum;
extern short* polyCount;
extern float* fovScaler;