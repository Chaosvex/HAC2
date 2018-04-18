#include "InitHooks.h"
#include "InputHandler.h"
#include "Patcher.h"
#include "EnginePointers.h"
#include "codefinder.h"
#include "misc.h"
#include "DebugHelper.h"
#include "FoVControl.h"
#include "BSPPatcher.h"
#include "SharedCaves.h"
#include "MotdRedirect.h"
#include "NetEvents.h"
#include "PatchGroup.h"
#include "MapControl.h"
#include "shared.h"
#include "mapCache.h"
#include "PasswordMasker.h"
#include "Direct3D.h"
#include "ParticleLimits.h"
#include "DisplayModes.h"
#include "GSRedirect.h"
#include "CustomChat.h"
#include "SettingsUI.h"
#include "ScoreboardTimer.h"
#include <sstream>
#include <string>
#include <d3d9.h>

namespace HookManager {

LAUNCHCB launchCB, quitCB;
void* launchArg = NULL, * quitArg = NULL;
std::vector <PatchGroup*> hooks;
PatchGroup* mapCacheHook();
PatchGroup* consoleFixHook();
PatchGroup* consoleInputHook();
PatchGroup* serverInfoHook();
PatchGroup* serverJoinHook();
PatchGroup* unknownHook();
PatchGroup* mapLoadHook();
PatchGroup* soundEventHook();
PatchGroup* lobbyFilterPatch();
PatchGroup* launchEventHook();
PatchGroup* terminalTypingHook();
PatchGroup* keyboardBufferHook();
PatchGroup* mouseBufferHook();
PatchGroup* fovHook();
PatchGroup* fovPatch();
PatchGroup* fovPatch2();
PatchGroup* exitHook();
PatchGroup* motdPushHook();
PatchGroup* postMapLoadHook();
PatchGroup* devmodePatch();
PatchGroup* playerEventHook();
PatchGroup* lobbyCustomMaps();
PatchGroup* hudCounterHook();
PatchGroup* resolutionChangeHook();
PatchGroup* d3dHooks();
PatchGroup* primaryD3DHook();
PatchGroup* secondaryD3DHook();
PatchGroup* tertiaryD3DHook();
PatchGroup* serverJoinHook();
PatchGroup* mapPathHook();
PatchGroup* exitHook();
PatchGroup* overlaySwitchHook();
PatchGroup* networkEventHook();
PatchGroup* buildChatPacketHook();
PatchGroup* engineTickHook();
void uninstall();
void installHooks();
void versionLocate();
void gameDetect();
void functionPointers();
void mapDetect();
void mapTablePointers();
void currentAddressPointers();
void manageMaps();
void createDirs();
void mapDirPatches();
void locateAddresses();
void installPatches();
void terminalHistory();
void copyMaps();
void fieldOfView();
void fovAddress();
void drawDistanceLocate();
void polyCountLocate();
void mapSectionsLocate();
void overlayToggleLocate();
void keyboardBufferLocate();
void mouseBufferLocate();
void cameraControlLocate();
void vehicleControlAddress();
void playerHeaderLocate();
void profilePathLocate();
void attentionBoxControlLocate();
void hudScaleLocate();
void resolutionLocate();
void objectsHeaderLocate();
void activeCamoFix();
void masterVolumeLocate();
void timeLocate();
void gravityLocate();

void launchCallback(LAUNCHCB callback, void* data) {
	launchCB = callback;
	launchArg = data;
}

void quitCallback(LAUNCHCB callback, void* data) {
	quitCB = callback;
	quitArg = data;
}

void install() {	
	gameDetect();
	locateAddresses();
	installPatches();
	installHooks();
}

void uninstall() {
	//EngineLimits::Particles::restore();
	GSRedirect::uninstall();
	EngineLimits::BSP::restore();
	DisplayModes::restore();
	SettingsUI::uninstall();
	scoreboard::restore();
	for(std::vector<int>::size_type i = 0; i != hooks.size(); i++) {
		delete hooks[i];
	}
	hooks.clear();
}

void launchHook() {
	if(strncmp(productType, "halor", 5) == 0) {
		alternateLoad();
	}

	try {
		hooks.emplace_back(d3dHooks());
		hooks.emplace_back(devmodePatch());
		hooks.emplace_back(mapPathHook());
			
	} catch(HookException& e) {
		MessageBox(NULL, e.what(), "An error occured while switching paths!", 0);
	}

	if(launchCB != nullptr) {
		launchCB(launchArg);
	}
}

void quitHook() {
	if(quitCB != nullptr) {
		quitCB(quitArg);
	}
}

void installPatches() {
	EngineLimits::BSP::patch();
	DisplayModes::install();
	SettingsUI::install();
	scoreboard::install();
	hooks.emplace_back(lobbyCustomMaps());
	hooks.emplace_back(lobbyFilterPatch());
	mapDirPatches();
	hooks.emplace_back(fovPatch());
	hooks.emplace_back(fovPatch2());
}

void installHooks() {
	if(strncmp(productType, "halom", 5) == 0) {
		hooks.emplace_back(mapCacheHook());
	}
	hooks.emplace_back(serverJoinHook());
	hooks.emplace_back(consoleFixHook());
	hooks.emplace_back(consoleInputHook());
	hooks.emplace_back(serverInfoHook());
	hooks.emplace_back(unknownHook());
	hooks.emplace_back(mapLoadHook());
	hooks.emplace_back(soundEventHook());
	hooks.emplace_back(launchEventHook());
	hooks.emplace_back(terminalTypingHook());
	hooks.emplace_back(keyboardBufferHook());
	hooks.emplace_back(mouseBufferHook());
	hooks.emplace_back(fovHook());
	hooks.emplace_back(motdPushHook());
	hooks.emplace_back(postMapLoadHook());
	hooks.emplace_back(hudCounterHook());
	hooks.emplace_back(resolutionChangeHook());
	hooks.emplace_back(overlaySwitchHook());
	hooks.emplace_back(playerEventHook());
	//hooks.emplace_back(serverJoinHook());
	hooks.emplace_back(exitHook());
	hooks.emplace_back(networkEventHook());
	hooks.emplace_back(buildChatPacketHook());
	hooks.emplace_back(engineTickHook());
	GSRedirect::install();
}

void locateAddresses() {
	overlayToggleLocate();
	mapSectionsLocate();
	mapTablePointers();
	terminalHistory();
	mapDetect();
	versionLocate();
	functionPointers();
	currentAddressPointers();
	fovAddress();
	drawDistanceLocate();
	polyCountLocate();
	keyboardBufferLocate();
	mouseBufferLocate();
	vehicleControlAddress();
	cameraControlLocate();
	playerHeaderLocate();
	profilePathLocate();
	attentionBoxControlLocate();
	hudScaleLocate();
	resolutionLocate();
	objectsHeaderLocate();
	activeCamoFix();
	masterVolumeLocate();
	timeLocate();
	gravityLocate();
}

PatchGroup* d3dHooks() {
	PatchGroup* group = primaryD3DHook();

	if(group) {
		return group;
	}

	group = secondaryD3DHook();

	if(group) {
		return group;
	}

	group = tertiaryD3DHook();
	
	if(group) {
		return group;
	}

	throw HookException("Direct3D hook failed!");
}

struct EnumData {
    DWORD processID;
    HWND window;
};

BOOL CALLBACK EnumProc(HWND window, LPARAM lParam) {
    EnumData& ed = *(EnumData*)lParam;
    DWORD processID;

    GetWindowThreadProcessId(window, &processID);

    if(ed.processID == processID) {
        ed.window = window;
        return FALSE;
    }

    return TRUE;
}

PatchGroup* secondaryD3DHook() {
	LPDIRECT3D9 d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	
	if(!d3d9) {
		OutputDebugString("Unable to create D3D9 instance");
		return NULL;
	}

	EnumData ed = { GetProcessId(GetCurrentProcess()), NULL };
	EnumWindows(EnumProc, reinterpret_cast<LPARAM>(&ed));

	if(!ed.window) {
		OutputDebugString("Unable to locate window to attach");
		d3d9->Release();
		return NULL;
	}

	LPDIRECT3DDEVICE9 device = NULL;
	D3DPRESENT_PARAMETERS params = {};
	params.Windowed = TRUE;
	params.hDeviceWindow = ed.window;

	HRESULT res = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, ed.window,
									 D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									 &params, &device);

	if(res != D3D_OK) {
		OutputDebugString("Unable to create temporary device");
		device->Release();
		d3d9->Release();
		return NULL;
	}

	PatchGroup *group = new PatchGroup();
	std::uintptr_t* vtable = *reinterpret_cast<std::uintptr_t**>(device);

	group->add(new CaveHook(vtable[42], 0, D3DHook::endScene, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalEndScene))); 
	group->add(new CaveHook(vtable[16], 0, D3DHook::reset, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalReset)));

	device->Release();
	d3d9->Release();

	if(!group->install()) {
		OutputDebugString("Unable to hook D3D functions");
		delete group;
		return NULL;
	}

	return group;
}

PatchGroup* tertiaryD3DHook() {	
	short signature[] = {0xC7, 0x06, -1, -1, -1, -1, 0x89, 0x86, -1, -1, -1, -1, 0x89, 0x86};
	DWORD address = FindCode(GetModuleHandle("d3d9.dll"), signature, sizeof(signature) / 2);
	
	if(address == NULL) {
		OutputDebugString("Unable to locate D3D9 signature");
		return NULL;
	}

	address += 2;

	PatchGroup *group = new PatchGroup();
	std::uintptr_t* vtable = reinterpret_cast<std::uintptr_t*>(*(std::uintptr_t*)address);

	group->add(new CaveHook(vtable[42], 0, D3DHook::endScene, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalEndScene))); 
	group->add(new CaveHook(vtable[16], 0, D3DHook::reset, CaveHook::CALL_DETOUR,
		reinterpret_cast<std::uintptr_t*>(&D3DHook::originalReset)));
		DebugHelper::DisplayAddress((DWORD)vtable[42], 16);

	if(!group->install()) {
		OutputDebugString("Unable to hook D3D functions");
		delete group;
		return NULL;
	}

	return group;
}

PatchGroup* primaryD3DHook() {
	PatchGroup *group = new PatchGroup();
	short endSig[] = {0xFF, 0x92, 0xA8, 0x00, 0x00, 0x00};
	short resetSig[] = {0xFF, 0x52, 0x40, 0x85, 0xC0};
	group->add(new CaveHook(endSig, sizeof(endSig) / 2, 0, D3DHook::endScene, CaveHook::CALL_NO_TP));
	group->add(new CaveHook(resetSig, sizeof(resetSig) / 2, 0, D3DHook::reset, CaveHook::CALL_NO_TP));
	
	if(!group->install()) {
		OutputDebugString("Unable to hook D3D call sites. Probably OpenSauce.");
		delete group;
		return NULL;
	}

	return group;
}

PatchGroup* overlaySwitchHook() {
	short signature[] = {0xE8, -1, -1, -1, -1, 0x84, 0xC0, 0x74, -1, 0x8B, 0x04, 0x24, 0x48};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, overlayTrigger, CaveHook::CALL_NO_TP,
		(std::uintptr_t*)&originalOverlayResolve));

	if(!group->install()) {
		delete group;
		throw HookException("Overlay event trigger hook failed!");
	}

	return group;
}

/*PatchGroup* serverJoinSuccessHook() {
	short signature[] = {0x1B, 0xC0, 0x83, 0xD8, 0xFF, 0x85, 0xC0, 0x74};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 5, (DWORD)serverJoinStub, &ContinueServerJoin));
	if(!group->install()) {
		delete group;
		throw HookException("Server join hook failed!");
	}
	return group;
}*/

PatchGroup* exitHook() {
	short signature[] = {0xFF, 0x15, -1, -1, -1, -1, 0xCC, 0x6A, 0x08, 0xE8, -1, -1, -1, -1, 0x59, 0xC3};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, quitHook, CaveHook::CALL_TP));
	if(!group->install()) {
		delete group;
		throw HookException("Game quit hook failed!");
	}
	return group;
}

PatchGroup* motdPushHook() {
	short signature[] = {0x8B, 0xC6, 0x50, 0x33, 0xD2, 0xE8};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, MotdRedirect, CaveHook::NAKED, &ContinueMotdLoad));
	if(!group->install()) {
		delete group;
		throw HookException("MotD hook failed!");
	}
	return group;
}

PatchGroup* hudCounterHook() {
	short signature[] = {0x8B, 0x44, 0x24, 0x3C, 0x8B, 0xD0};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, HUDCounterScale, CaveHook::NAKED, &ContinueCounterRender));
	if(!group->install()) {
		delete group;
		throw HookException("HUD counter hook failed!");
	}
	return group;
}

PatchGroup* resolutionChangeHook() {
	short signature[] = {0x6A, 0x00, 0x6A, 0x65, 0x6A, 0x08, 0x6A, 0x08, 0x50, 0xFF, 0x51};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, resolutionNotify, CaveHook::CALL_TP));
	if(!group->install()) {
		delete group;
		throw HookException("Resolution change hook failed!");
	}
	return group;
}

PatchGroup* keyboardBufferHook() {
	short signature[] = {0x81, 0xF9, 0xFF, 0x00, 0x00, 0x00, 0x7E, -1, -1, 0xFF, 0x00, 0x00, 0x00, 0x88, -1, 0x40, 0x4A};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, keyboardRead, CaveHook::CALL_TP));
	if(!group->install()) {
		delete group;
		throw HookException("Keyboard buffer hook failed!");
	}
	return group;
}

PatchGroup* postMapLoadHook() {
	short signature[] = {0x25, 0xFF, 0xFF, 0x00, 0x00, 0xC1, 0xE0, 0x05, 0x8B, 0x44, 0x08, 0x14, 0x8B, 0x88, 0xA4, 0x05,
						 0x00, 0x00, 0x85, 0xC9};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, postMapLoad, CaveHook::CALL_TP));
	if(!group->install()) {
		delete group;
		throw HookException("Post-load hook failed!");
	}
	return group;
}

PatchGroup* playerEventHook() {
	short signature[] = {0x6A, 0x00, 0x51, 0x52, 0x50, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x10};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, PlayerEventStub, CaveHook::JMP_TP, &ContinuePlayerEvent));
	if(!group->install()) {
		delete group;
		throw HookException("Player killed hook failed!");
	}
	return group;
}

PatchGroup* announcerMuteHook() { //todo - better name when I know what this is
	short signature[] = {0x8B, 0x84, 0x24, 0xA0, 0x00, 0x00, 0x00, 0xFF, 0x24, 0x85};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, postMapLoad, CaveHook::CALL_TP));
	if(!group->install()) {
		delete group;
		throw HookException("Announcer mute hook failed!");
	}
	return group;
}

PatchGroup* mouseBufferHook() {
	short signature[] = {0xBB, 0x08, 0x00, 0x00, 0x00, 0x8A, 0x0E, 0x66, 0x8B, 0x17, 0xC0, 0xE9, 0x07, 0x84, 0xC9};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, mouseRead, CaveHook::CALL_TP));
	if(!group->install()) {
		delete group;
		throw HookException("Mouse buffer hook failed!");
	}
	return group;
}

void announcerSkipLocate() {
	short signature[] = {0xC6, 0x44, 0x24, 0x0F, 0x00, 0x8B, 0x94, 0x24, 0xA8, 0x00, 0x00, 0x00};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address++;
		pKeyboardBuffer = (BYTE*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate announcer skip!");
	}
}

void hudScaleLocate() {
	std::vector<DWORD> address;
	short signature1[] = {0xC7, 0x84, 0x24, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F};
	address.emplace_back(FindCode(GetModuleHandle(0), signature1, sizeof(signature1) / 2));

	if(address[0] != NULL) {
		address[0] += 7;
		pGametypeIconBGWidth = (float*)address[0];
		DWORD old = 0;
		VirtualProtect(pGametypeIconBGWidth, 4, PAGE_EXECUTE_READWRITE, &old);
	} else {
		throw HookException("Failed to locate gametype icon!");
	}

	short signature2[] = {0x66, 0xC7, 0x44, 0x24, 0x70, 0xBD, 0x01};
	address.emplace_back(FindCode(GetModuleHandle(0), signature2, sizeof(signature2) / 2));

	if(address[1] != NULL) {
		address[1] += 5;
		pGametypeIconBGXPos = (uint16_t*)address[1];
		DWORD old = 0;
		VirtualProtect(pGametypeIconBGXPos, 2, PAGE_EXECUTE_READWRITE, &old);
	} else {
		throw HookException("Failed to locate gametype icon!");
	}

	short signature3[] = {0x66, 0xC7, 0x44, 0x24, 0x30, 0xC8, 0x01, 0x66, 0xC7, 0x44, 0x24, 0x32, 0x06, 0x00};
	address.emplace_back(FindCode(GetModuleHandle(0), signature3, sizeof(signature3) / 2));

	short signature4[] = {0x66, 0xC7, 0x44, 0x24, 0x30, 0xCA, 0x01};
	address.emplace_back(FindCode(GetModuleHandle(0), signature4, sizeof(signature4) / 2));

	short signature5[] = {0x66, 0xC7, 0x44, 0x24, 0x30, 0xC8, 0x01, 0x66, 0xC7, 0x44, 0x24, 0x32, 0x07, 0x00};
	address.emplace_back(FindCode(GetModuleHandle(0), signature5, sizeof(signature5) / 2));

	short signature6[] = {0x66, 0xC7, 0x44, 0x24, 0x30, 0xC5, 0x01, 0x66, 0xC7, 0x44, 0x24, 0x32, 0x06, 0x00};
	address.emplace_back(FindCode(GetModuleHandle(0), signature6, sizeof(signature6) / 2));

	short signature7[] = {0x66, 0xC7, 0x44, 0x24, 0x30, 0xC9, 0x01};
	address.emplace_back(FindCode(GetModuleHandle(0), signature7, sizeof(signature7) / 2));
		
	for(auto i = address.begin() + 2; i != address.end(); i++) {
		if((*i) != NULL) {
			(*i) += 5;
			pGametypeIconOffsets.emplace_back((uint16_t*)(*i));
			DWORD old = 0;
			VirtualProtect((void*)(*i), 2, PAGE_EXECUTE_READWRITE, &old);
		} else {
			throw HookException("Failed to locate gametype icon!");
		}
	}

	address.clear();

	short signature8[] = {0xC7, 0x44, 0x24, 0x60, 0x00, 0x00, 0x80, 0x3F, 0xD9, 0x5C, 0x24, 0x38};
	address.emplace_back(FindCode(GetModuleHandle(0), signature8, sizeof(signature8) / 2));

	short signature9[] = {0xC7, 0x44, 0x24, 0x78, 0x00, 0x00, 0x80, 0x3F, 0xD9, 0x5C, 0x24, 0x4C};
	address.emplace_back(FindCode(GetModuleHandle(0), signature9, sizeof(signature9) / 2));

	for(auto i = address.begin(); i != address.end(); i++) {
		if((*i) != NULL) {
			(*i) += 4;
			pRadarMask.emplace_back((float*)(*i));
			DWORD old = 0;
			VirtualProtect((void*)(*i), 4, PAGE_EXECUTE_READWRITE, &old);
		} else {
			throw HookException("Failed to locate gametype icon!");
		}
	}
}

void keyboardBufferLocate() {
	short signature[] = {0xB8, -1, -1, -1, -1, 0xBA, 0x6D, 0x00, 0x00, 0x00, 0x8D, 0x49, 0x00, 0x80, -1, 0x6D, 0x01};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address++;
		pKeyboardBuffer = (BYTE*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate keyboard buffer");
	}
}

void resolutionLocate() {
	short signature[] = {0xA3, -1, -1, -1, -1, 0x89, 0x0D, -1, -1, -1, -1, 0xEB, 0x6C};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address++;
		pResolution = (EngineTypes::Resolution*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate resolution information");
	}
}

void objectsHeaderLocate() {
	short signature[] = {0x8B, 0x43, 0x18, 0x8B, 0x15, -1, -1, -1, -1};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 5;
		pObjectsHeader = (EngineTypes::ObjectsHeader**)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate resolution information");
	}
}

void mouseBufferLocate() {
	short signature[] = {0xA3, -1, -1, -1, -1, 0xA3, -1, -1, -1, -1, 0xA3, -1, -1, -1, -1, 0xA3, -1, -1, -1, -1, 0xA3,
						 -1, -1, -1, -1, 0xA3, -1, -1, -1, -1, 0xB9, 0x28};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address++;
		pMouseBuffer = (BYTE*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate mouse buffer");
	}
}

void playerHeaderLocate() {
	short signature[] = {0x8B, -1, -1, -1, -1, -1, 0x25, 0xFF, 0xFF, 0x00, 0x00, 0x56, 0x8B, 0x72, 0x34};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);
	if(address != NULL) {
		address += 2;
		pPlayerHeader = (EngineTypes::PlayerHeader**)*(DWORD**)address;
	} else {
		throw HookException("Failed to locate player header");
	}
}

void cameraControlLocate() {
	short signature[] = {0x88, 0x1D, -1, -1, -1, -1, 0x88, 0x1D, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x8A, -1, -1,
						 0x2C, 0x01, 0x00, 0x00};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 2;
		pCameraControl = (EngineTypes::CameraControl*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate camera control structures");
	}
}

void mapSectionsLocate() {
	short signature[] = {0xC7, 0x05, -1, -1, -1, -1, -1, -1, -1, -1, 0xFF, 0xD6};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 6;
		pTagTableHeader = (EngineTypes::TagTableHeader*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate tag table");
	}
}

void overlayToggleLocate() {
	short signature[] = {0xA3, -1, -1, -1, -1, 0xA3, -1, -1, -1, -1, 0x66, 0xA3, -1, -1, -1, -1, 0x66, 0xA3, -1, -1, -1,
						 -1, 0x89, 0x0D};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address++;
		overlayControl = (DWORD*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate section table");
	}
}

void drawDistanceLocate() {
	short signature[] = {0xA1, -1, -1, -1, -1, 0x8B, 0x15, -1, -1, -1, -1, 0x89, -1, 0x98, 0x00, 0x00, 0x00, 0xA0};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address++;
		pDrawDistance = (float*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate draw distance");
	}
}

void polyCountLocate() {
	short signature[] = {0x66, 0xFF, 0x05, -1, -1, -1, -1, 0x8B, 0x47, -1, 0x83, 0xC6, 0x04, 0x45};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 3;
		polyCount = (short*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate poly count");
	}
}

void activeCamoFix() {
	short signature[] = {0x8A, 0x0D, -1, -1, -1, -1, 0x84, 0xC9, 0x75, 0x18};
	std::uintptr_t address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 2;
		activeCamoDisable = (std::uint8_t*)*(std::uintptr_t*)address;
	} else {
		throw HookException("Failed to locate active camo shader toggle");
	}
}

void masterVolumeLocate() {
	short signature[] = {0xBE, -1, -1, -1, -1, 0x8D, 0x7C, 0x24, 0x10, 0xF3, 0xA5, 0x8B, 0x42, 0x34};
	std::uintptr_t address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 1;
		masterVolume = (std::uint8_t*)*(std::uintptr_t*)address;
		masterVolume += 2936; //that's some offset
	} else {
		throw HookException("Failed to locate master volume");
	}
}

void timeLocate() {
	short signature[] = {0xD8, 0x0D, -1, -1, -1, -1, 0x83, 0xEC, 0x08, 0xD9, 0x5C, 0x24, 0x08};
	std::uintptr_t address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 2;
		tickrate = (float*)*(std::uintptr_t*)address;
	} else {
		throw HookException("Failed to locate tickrate signature");
	}
}

void gravityLocate() {
	short signature[] = {0xD8, 0x25, - 1, -1, -1, -1, 0xD9, 0x5C, 0x24, 0x24, 0xD9, 0x44, 0x24, 0x1C };
	std::uintptr_t address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 2;
		gravity = (float*)*(std::uintptr_t*)address;
	} else {
		throw HookException("Failed to locate tickrate signature");
	}
}

PatchGroup* fovPatch() {
	short signature[] = {0xF6, 0xC4, 0x41, 0x75, 0x0D, 0xD9, 0x5C, 0x24, 0x18, 0x66, 0xC7, 0x44, 0x24, 0x16, 0x02, 0x00, 0xEB};
	BYTE replacement[] = {0x31, 0xC0, 0x90};
	PatchGroup *group = new PatchGroup();
	group->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, 3));
	if(!group->install()) {
		delete group;
		throw HookException("Failed to apply FoV zoom rendering fix!");
	}
	return group;
}

PatchGroup* fovPatch2() {
	short signature[] = {0xF6, 0x42, 0x40, 0x01, 0x0F, 0x85};
	BYTE replacement[] = {0x85, 0xC0, 0x89, 0xC0};
	PatchGroup *group = new PatchGroup();
	group->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, 4));
	if(!group->install()) {
		delete group;
		throw HookException("Failed to apply FoV zoom rendering fix!");
	}
	return group;
}

void fovAddress() {
	short signature[] = {0xD8, 0x0D, -1, -1, -1, -1, 0xDD, 0x05, -1, -1, -1, -1, 0xD9, 0xF3, 0xDC, 0xC0, 0xD9, 0x9F,
						 0x80, 0x00, 0x00, 0x00, 0x0F};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 2;
		fovScaler = (float*)*(DWORD*)address;
		DWORD old = 0;
		VirtualProtect(fovScaler, 4, PAGE_READWRITE, &old);
	} else {
		throw HookException("Failed to locate FoV scaler - falling back to hook");
	}
}

void vehicleControlAddress() {
	short signature[] = {0x74, 0x02, 0x32, -1, 0x8B, -1, 0x14, 0x01, 0x00, 0x00, 0x83, 0xFA, 0xFF};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		DWORD old = 0;
		VehicleControl = (char*)address;
		VirtualProtect(VehicleControl, 4, PAGE_EXECUTE_READWRITE, &old);
	} else {
		throw HookException("Failed to locate MTV patch location");
	}
}

PatchGroup* fovHook() {
	short signature[] = {0x89, 0x93, 0xAC, 0x00, 0x00, 0x00, 0x5B, 0x83, 0xC4, 0x14};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, FoV::redirect, CaveHook::NAKED, &ContinueFoV));

	if(!group->install()) {
		delete group;
		throw HookException("FoV hook failed!");
	}

	return group;
}

void terminalHistory() {
	short signature1[] = {0x05, -1, -1, -1, -1, 0x8D, 0x64, 0x24, 0x00, 0x8A, 0x0A, 0x42, 0x88, 0x08, 0x40, 0x3A, 0xCB, 0x75};
	DWORD pHistBuff = FindCode(GetModuleHandle(0), signature1, sizeof(signature1) / 2);

	short signature2[] = {0x66, 0xA3, -1, -1, -1, -1, 0x0F, 0xBF, 0xC0, 0x69, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x8B, 0xD7, 0x05};
	DWORD pHistBuffLen = FindCode(GetModuleHandle(0), signature2, sizeof(signature2) / 2);

	if(pHistBuff != NULL || pHistBuffLen != NULL) {
		pHistBuff++;
		pHistBuffLen += 2;
		histBuff = (char*)*(DWORD*)pHistBuff;
		histBuffLen = (short*)*(DWORD*)pHistBuffLen;
	} else {
		throw HookException("Unable to locate terminal buffer pointers.");
	}
}

PatchGroup* launchEventHook() {
	short signature[] = {0x83, 0xC4, 0x04, 0x84, 0xC0, 0x74, -1, 0x85, 0xF6, 0x74, -1, 0x6A, 0x7F};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, launchHook, CaveHook::CALL_TP));

	if(!group->install()) {
		delete group;
		throw HookException("Launch callback hook failed!");
	}

	return group;
}

PatchGroup* terminalTypingHook() {
	short signature[] = {0x88, 0x04, 0x11, 0x83, 0xC4, 0x0C, 0x66, 0xFF, 0x43, 0x06, 0x8D, 0x7B, 0x06};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, PasswordMaskerStub, CaveHook::NAKED, &ContinueTerminalInput));
	
	if(!group->install()) {
		delete group;
		throw HookException("Terminal typing hook failed!");
	}

	return group;
}

/*
 * Replaces pointers contained in CE's instructions with a pointer
 * to a larger buffer of our own to allow us to redirect map
 * loading.
 */
void mapDirPatches() {
	mapSearchPath = static_cast<char*>(calloc(MAX_PATH_SIZE, 1));
	
	if(mapSearchPath == NULL) {
		throw HookException("Memory allocation failed!");
	}

	strcpy_s(mapSearchPath, MAX_PATH_SIZE, "maps\\");

	short signature1[] = {0x68, -1, -1, -1, -1, 0x68, -1, -1, -1, -1, 0x8D, 0x4C, 0x24, 0x18, 0x68, -1, -1, -1, -1, 0x51,
						  0x32, 0xDB, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x14, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x03, 0x6A,
						  0x00, 0x6A, 0x01, 0x68};
	short signature2[] = {0x68, -1, -1, -1, -1, 0xB9, 0x00, 0x02, 0x00, 0x00, 0x68, -1, -1, -1, -1, 0x68, -1, -1, -1, -1,
						  0x8B, 0xD8, 0x0F, 0xBF, 0xF3, 0x69, 0xF6, 0x0C, 0x08, 0x00, 0x00};

	PatchGroup *group = new PatchGroup();
	group->add(new PatchHook(signature1, sizeof(signature1) / 2, 1, reinterpret_cast<std::uint8_t*>(&mapSearchPath), 4));
	group->add(new PatchHook(signature2, sizeof(signature2) / 2, 1, reinterpret_cast<std::uint8_t*>(&mapSearchPath), 4));
		
	if(!group->install()) {
		delete group;
		throw HookException("Failed to patch map search locations!");
	}

	hooks.emplace_back(group);
}

PatchGroup* devmodePatch() {
	short signaturePC[] = {0x32, 0xC0, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0x66, 0xA1};
	short signatureCE[] = {0x32, 0xC0, 0x5E, 0xC3, 0x8A, 0xC1, 0xC0, 0xE8, 0x02};
	short* signature = (strncmp(productType, "halom", 5) == 0)? signatureCE : signaturePC;

	DWORD call = FindCode(GetModuleHandle(0), signature, sizeof(signaturePC) / 2);
	
	if(call == NULL) {
		throw HookException("Devmode patch failed!");
	}
	
	BYTE replacement[] = {0xB0, 0x01};
	PatchGroup *group = new PatchGroup();
	group->add(new PatchHook(call, 0, replacement, 2));

	if(!group->install()) {	
		delete group;
		throw HookException("Devmode patch failed!");
	}
	
	return group;
}

/*
 * This hook allows HAC to intercept map load requests, enabling the game to load map files
 * from directories other than the default.
 */
PatchGroup* mapPathHook() {
	short signature[] = {0x81, 0xEC, 0x04, 0x01, 0x00, 0x00, 0x53, 0x57, 0x50, 0x68, -1, -1, -1, -1, 0x68, -1, -1, -1,
						 -1, 0x8D, 0x4C, 0x24, 0x18};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, SearchPathSwitchCC, CaveHook::JMP_TP, &BackToPathLoad));
	if(!group->install()) {
		delete group;
		throw HookException("Map path hook failed!");
	}
	return group;
}

void functionPointers() {
	short signature1[] = {0x66, 0x3D, 0xFF, 0xFF, 0x74, -1, 0x8B, 0x15, -1, -1, -1, -1, 0x56, 0x57, 0x0F, 0xBF, 0xF8, 0x69,
						  0xFF, 0x60, 0x04, 0x00, 0x00, 0x03, 0xFA, 0x6A, 0x00};
	HUDPrint = FindCode(GetModuleHandle(0), signature1, sizeof(signature1) / 2);

	short signature2[] = {0x83, 0xEC, 0x10, 0x8A, 0x4C, 0x24, 0x14, 0x55, 0x6A, 0x00, 0x6A, 0x01, 0x6A, 0x00, 0x88,
						  0x4C, 0x24, 0x18};
	ChatSend = FindCode(GetModuleHandle(0), signature2, sizeof(signature2) / 2);

	short signature3[] = {0x83, 0xEC, 0x10, 0x57, 0x8B, 0xF8, 0xA0, -1, -1, -1, -1, 0x84, 0xC0, 0xC7, 0x44, 0x24, 0x04,
						  0x00, 0x00, 0x80, 0x3F};
	ConsolePrint = FindCode(GetModuleHandle(0), signature3, sizeof(signature3) / 2);

	short signature4[] = {0x83, 0xEC, 0x6C, 0xA1, -1, -1, -1, -1, 0x8B, 0x4C, 0x24, 0x70, 0x89, 0x44, 0x24, 0x68, 0x33, 0xC0};
	GenMD5 = FindCode(GetModuleHandle(0), signature4, sizeof(signature4) / 2);

	short signature5[] = {0x8B, 0x0D, -1, -1, -1, -1, 0x81, 0xEC, 0x00, 0x08, 0x00, 0x00, 0x53, 0x55, 0x8B, 0x2D};
	LoadMap = FindCode(GetModuleHandle(0), signature5, sizeof(signature5) / 2);

	short signature6[] = {0x66, 0x83, 0x3D, -1, -1, -1, -1, 0x02, 0x74, -1, 0xC6, 0x44, 0x24, 0x04, 0x00, 0x8A, 0x86, -1,
						  -1, -1, -1, 0x84, 0xC0};
	SoundPlay = FindCode(GetModuleHandle(0), signature6, sizeof(signature6) / 2);

	short signature7[] = {0x81, 0xEC, 0x1C, 0x01, 0x00, 0x00, -1, 0x8B, -1, 0x33, 0xC0, 0x89, 0x44, 0x24, 0x08, 0x33, 0xC9, 0x89};
	Connect = FindCode(GetModuleHandle(0), signature7, sizeof(signature7) / 2);

	short signature8[] = {0x8A, 0x07, 0x81, 0xEC, 0x00, 0x05, 0x00, 0x00, 0x3C, 0x3B, 0x74};
	CommandExecute = FindCode(GetModuleHandle(0), signature8, sizeof(signature8) / 2);

	short signature9[] = {0x51, 0xA0, -1, -1, -1, -1, 0x53, 0x33, 0xDB, 0x3A, 0xC3, 0x56};
	DisplayMenu = FindCode(GetModuleHandle(0), signature9, sizeof(signature9) / 2);

	short signature10[] = {0xA1, -1, -1, -1, -1, 0x83, 0xEC, 0x08, 0x83, 0xF8, 0x08};
	ChatLocalSend = FindCode(GetModuleHandle(0), signature10, sizeof(signature10) / 2);

	short signature11[] = {0x0F, 0x84, -1, -1, -1, -1, 0x53, 0xE8, -1, -1, -1, -1, 0x8B, 0xD8, 0x83, 0xFB, 0xFF};
	GetLocalPlayerIndex = 0x004AE5D0; // TODO

	if(HUDPrint == NULL || LoadMap == NULL || ChatSend == NULL || ConsolePrint == NULL
		|| GenMD5 == NULL || SoundPlay == NULL || Connect == NULL || CommandExecute == NULL
		|| DisplayMenu == NULL || ChatLocalSend == NULL || GetLocalPlayerIndex == NULL) {
		throw HookException("Unable to locate functions!");
	}
}

void mapTablePointers() {
	short signature[] = {0x3B, 0x05, -1, -1, -1, -1, 0x7D, 0x24, 0x8B, 0x0D};
	DWORD pMapTableSize = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);
	
	if(pMapTableSize == NULL) {
		throw HookException("Unable to locate map table pointers!");
	}

	pMapTableSize += 2;
	MapTableSize = (DWORD*)*(DWORD*)pMapTableSize;
	pMapTable = (EngineTypes::MapEntry**)(MapTableSize - 1);
}

void currentAddressPointers() {
	short signature1[] = {0xC6, 0x05, -1, -1, -1, -1, 0x00, 0x66, 0x8B, 0x48, 0x10, 0x66, 0x83, 0xF9, 0x04, 0x75, -1,
						  0x0F, 0xB7, 0x48, 0x12};
	DWORD pCurrAddress = FindCode(GetModuleHandle(0), signature1, sizeof(signature1) / 2);

	short signature2[] = {0xBA, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x66, 0xA1, -1, -1, -1, -1, 0x66, 0x25, 0xF9,
						  0xFF, 0x83, 0xCA, 0xFF, 0x66, 0xA3, -1, -1, -1, -1, 0xB9, 0x0C, 0x00, 0x00, 0x00, 0x33, 0xC0, 0xBF};
	DWORD pServName = FindCode(GetModuleHandle(0), signature2, sizeof(signature2) / 2);

	if(pCurrAddress != NULL || pServName != NULL) {
		pCurrAddress += 2;
		pServName++;
		serverAddress = (char*)*(DWORD*)pCurrAddress;
		serverName = (wchar_t*)*(DWORD*)pServName;
	} else {
		throw HookException("Unable to locate server address information!");
	}
}

/*
 * Extracts the static address of the game's Gamespy product ID from the code.
 * This is used to determine whether or not we care about applying CE specific
 * hooks and patches, as well as for use in version changing.
 */
void gameDetect() {
	short signature[] = {0xB8, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x04, 0xE8, -1, -1, -1, -1, 0xE8,
						 -1, -1, -1, -1, 0x8B, 0x45, 0xE0, 0xE8, -1, -1, -1, -1, 0xA1};
	std::uintptr_t pProduct = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(pProduct != NULL) {
		pProduct++;
		productType = (char*)*(std::uintptr_t*)pProduct;
	} else {
		throw HookException("Unable to locate GS ID information");
	}
}

void mapDetect() {
	short signature[] = {0xB8, -1, -1, -1, -1, 0x8D, 0x50, 0x01, 0x8A, 0x08, 0x40, 0x84, 0xC9, 0x75, -1, 0x2B, 0xC2,
						 0x83, 0xF8, 0x1F};
	DWORD pMap = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(pMap != NULL) {
		pMap++;
		pCurrentMap = (char*)*(DWORD*)pMap;
	} else {
		throw HookException("Failed to locate map information!");
	}
}

PatchGroup* mapCacheHook() {
	short beginCalculateChecksum[] = {0x83, 0xEC, 0x1C, 0x53, 0x55, 0x8B, 0x6C, 0x24, 0x28, 0x56, 0x57, 0x6A, 0x5C,
									  0x55, 0xC6, 0x44, 0x24, 0x1B, 0x00, 0xE8};
	short grabCalculatedChecksum[] = {0xFF, 0x15, -1, -1, -1, -1, 0x8B, -1, 0x24, -1, 0x8B, -1, 0x24, -1, 0x89, 0x0A};
	short writeCache[] = {0x56, 0xFF, 0x15, -1, -1, -1, -1, 0x5F, 0x5E, 0x81, 0xC4, 0x40, 0x01, 0x00, 0x00};

	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(beginCalculateChecksum, sizeof(beginCalculateChecksum) / 2, 0, LoadFix, CaveHook::NAKED, &DoChecksum));
	group->add(new CaveHook(writeCache, sizeof(writeCache) / 2, 0, WriteCacheCC, CaveHook::JMP_TP, &BackToLoading));
	group->add(new CaveHook(grabCalculatedChecksum, sizeof(grabCalculatedChecksum) / 2, 14, AddChecksumCC, CaveHook::JMP_TP, &ContinueChecksum));

	if(!group->install()) {
		delete group;
		throw HookException("Map caching and checksum hooks failed");
	}
	 
	// temporary workaround because OpenSauce etc
	short cacheLoader[] = {0x81, 0xEC, 0x40, 0x01, 0x00, 0x00, 0x56, 0xBE};
	short cacheLoader2[] = {0xE9, -1, -1, -1, -1, 0x00, 0x56, 0xBE};

	PatchGroup* test = new PatchGroup();
	test->add(new CaveHook(cacheLoader, sizeof(cacheLoader) / 2, 0, loadChecksums, CaveHook::CALL_TP));

	if(!test->install()) {
		delete test;
		test = new PatchGroup();
		test->add(new CaveHook(cacheLoader2, sizeof(cacheLoader2) / 2, 0, loadChecksums, CaveHook::CALL_TP));

		if(!test->install()) {
			delete test;
			delete group;
			throw HookException("Map caching and checksum hooks failed");
		}
	}


	return group;
}

PatchGroup* mapLoadHook() {
	short signature[] = {0x6A, 0x01, 0x55, 0x51, 0x8D, 0x44, 0x24};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, mapLoadStub, CaveHook::CALL_TP));
	if(!group->install()) {
		delete group;
		throw HookException("Map hook failed!");
	}
	return group;
}

PatchGroup* soundEventHook() {
	short signature[] = {0xC6, 0x44, 0x24, 0x04, 0x00, 0x8A, 0x86};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, SoundEventStub, CaveHook::NAKED, &ContinuePlaySound));
	if(!group->install()) {
		delete group;
		throw HookException("Event hook failed!");
	}
	return group;
}

PatchGroup* consoleFixHook() {
	short signature[] = {0x69, 0xC0, 0x24, 0x01, 0x00, 0x00, 0x8B, 0x8C, 0x30};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, ConsoleFadeFixCC, CaveHook::NAKED, &BackToHalo));
	if(!group->install()) {
		delete group;
		throw HookException("Terminal text fix hook failed");
	}
	return group;
}

PatchGroup* consoleInputHook() {
	short signature[] = {0x8A, 0x07, 0x81, 0xEC, 0x00, 0x05, 0x00, 0x00, 0x3C, 0x3B, 0x74};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, ConsoleReadCC, CaveHook::NAKED, &ProcessCommands));
	if(!group->install()) {
		delete group;
		throw HookException("Console input hook failed");
	}
	return group;
}

PatchGroup* serverInfoHook() {
	short signature[] = {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0xB8, 0x2C, 0x20, 0x00, 0x00, 0xE8}; //r A0
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, SavePasswordStub, CaveHook::JMP_TP, &BackToJoining));
	if(!group->install()) {
		delete group;
		throw HookException("Server information hook failed");
	}
	return group;
}

PatchGroup* serverJoinHook() {
	short signature[] = {0xB9, 0xFF, 0x07, 0x00, 0x00, 0xBE, -1, -1, -1, -1, 0x8D, 0x7C, 0x24, 0x38, 0xF3, 0xA5};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, serverJoinCave, CaveHook::NAKED, &BackToConnecting));
	if(!group->install()) {
		delete group;
		throw HookException("Server join hook failed");
	}
	return group;
}

PatchGroup* unknownHook() {
	short signature[] = {0xB9, 0x0C, 0x00, 0x00, 0x00, 0x33, 0xC0, 0xF3, 0xAB};
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, TestCC, CaveHook::NAKED, &TestCont));
	if(!group->install()) {
		delete group;
		throw HookException("Unknown hook failed - may or may not be a problem");
	}
	return group;
}

/*
 * Locates the static address that contains the build number
 */
void versionLocate() {
	short signature[] = {0x68, -1, -1, -1, -1, 0x8B, 0xC3, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x08, 0x66, 0x3B, 0xC7,
						 0x74, -1, 0xC6, 0x44, 0x24, 0x18, 0x00};
	versionNum = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(versionNum != NULL) {
		versionNum++;
		DWORD oldProtect = 0;
		VirtualProtect((void*)versionNum, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	} else {
		throw HookException("Failed to locate build information");
	}
}

void profilePathLocate() {
	short signature[] = {0x33, 0xC0, 0xB9, 0x41, 0x00, 0x00, 0x00, 0xBF, -1, -1, -1, -1, 0xF3, 0xAB, 0xAA};
	DWORD address = FindCode(GetModuleHandle(0), signature, sizeof(signature) / 2);

	if(address != NULL) {
		address += 8;
		pProfilePath = (char*)*(DWORD*)address;
	} else {
		throw HookException("Failed to locate build information");
	}
}

void attentionBoxControlLocate() {
	short signature1[] = {0x66, 0xA1, -1, -1, -1, -1, 0x83, 0xCE, 0xFF, 0x66, 0x3B, 0xC6};
	DWORD indexAddr = FindCode(GetModuleHandle(0), signature1, sizeof(signature1) / 2);

	short signature2[] = {0x88, -1, -1, -1, -1, -1, 0xA2, -1, -1, -1, -1, 0x5B, 0x59, 0xC3};
	DWORD controlAddr = FindCode(GetModuleHandle(0), signature2, sizeof(signature2) / 2);

	if(indexAddr != NULL || controlAddr != NULL) {
		indexAddr += 2;
		controlAddr += 2;
		stringIndex = (int16_t*)*(DWORD*)indexAddr;
		displayAttentionBox = (bool*)*(DWORD*)controlAddr;
	} else {
		throw HookException("Failed to locate attention box controls");
	}
}

/*
 * Patches the server list filter code to remove version based filtering
 */
PatchGroup* lobbyFilterPatch() {
	short signature[] = {0x74, 0x7B, 0xA0, -1, -1, -1, -1, 0x84, 0xC0, 0x74, 0x44, 0x0F, 0xB6, 0xC0};
	BYTE replacement[] = {0x90, 0x90};
	PatchGroup *group = new PatchGroup();
	group->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, 2));
	if(!group->install()) {
		delete group;
		throw HookException("Failed to patch server filtering function");
	}
	return group;
}

/*
 * Allows HPC to display custom maps
 */
PatchGroup* lobbyCustomMaps() {
	PatchGroup *group = new PatchGroup();

	if(strncmp(productType, "halor", 5) == 0) {
		short signature[] = {0x75, 0x0A, 0x32, 0xC0, 0x5F, 0x5D, 0x5E, 0x5B, 0x83, 0xC4, 0x10};
		BYTE replacement[] = {0xEB};
		group->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	} else {
		short signature[] = {0x8A, 0x84, 0x24, 0xCD, 0x0C, 0x00, 0x00, 0x88, 0x0D};
		BYTE replacement[] = {0xB0, 0x01, 0x90, 0x90, 0x90, 0x90, 0x90};
		group->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	if(!group->install()) {
		delete group;
		throw HookException("Failed to patch custom map function");
	}
	
	return group;
}

PatchGroup* networkEventHook() {
	PatchGroup *group = new PatchGroup();

	short signature[] = {0x8B, 0x44, 0x24, 0x0C, 0x83, 0xF8, 0x04, 0x75};
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, NetEvents::netEventsStub,
	                        CaveHook::NAKED, &NetEvents::BackToNetHandler));

	if(!group->install()) {
		delete group;
		throw HookException("Failed to install network events hook");
	}
	
	return group;
}

PatchGroup* engineTickHook() {
	PatchGroup *group = new PatchGroup();

	short signature[] = { 0xA1, -1, -1, -1, -1, 0x8B, 0x50, 0x14, 0x8B, 0x48, 0x0C, 0x83, 0xC4, 0x04, 0x42, 0x41, 0x4E, 0x4F, 0x89, 0x50, 0x14, 0x89, 0x48, 0x0C }; // A1????????8B50148B480C83C40442414E4F89501489480C
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, engineTick, CaveHook::CALL_TP, &BackToEngineTick));

	if(!group->install()) {
		delete group;
		throw HookException("Failed to install network events hook");
	}
	
	return group;
}

PatchGroup* buildChatPacketHook() {
	PatchGroup *group = new PatchGroup();

	std::uintptr_t originalFunc; // don't care

	short signature[] = {0xE8, -1, -1, -1, -1, 0x89, 0x44, 0x24, 0x0C, 0x8B, 0x44, 0x24, 0x18, 0x83, 0xC4, 0x04};
	group->add(new CaveHook(signature, sizeof(signature) / 2, 0, NetEvents::buildChatPacket,
	                        CaveHook::CALL_NO_TP, &originalFunc));

	if(!group->install()) {
		delete group;
		throw HookException("Failed to install network events hook");
	}
	
	return group;
}


};