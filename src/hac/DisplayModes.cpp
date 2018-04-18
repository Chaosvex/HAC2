#include "DisplayModes.h"
#include "Patcher.h"
#include "PatchGroup.h"
#include "console.h"
#include "Direct3D.h"
#include "Codefinder.h"
#include "DebugHelper.h"
#include <memory>
#include <Windows.h>
#include <cstdint>

namespace DisplayModes {

std::unique_ptr<PatchGroup> patches = NULL;
HWND* window = NULL;
bool locateWindowHandle();
void setWindowed();
void setWindowedFull();
void setFullscreen();
LONG prevStyle;

enum DISPLAY_MODES {
	WINDOWED, WINDOWED_FULLSCREEN
} displayMode = WINDOWED;

void install() {
	patches = std::unique_ptr<PatchGroup>(new PatchGroup());

	{ //bypass the resolution limit checks
		short signature[] = {0x74, 0x0A, 0x8B, 0x15, -1, -1, -1, -1, 0x85, 0xD2};
		std::uint8_t replacement[] = {0xEB, 0x54};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, 2));
	}

	{ //allow the window width to equal desktop width
		short signature[] = {0x73, 0x06, 0x3B, 0x5C, 0x24, 0x30};
		std::uint8_t replacement[] = {0x77, 0x06};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, 2));
	}

	{ //allow the window height to equal desktop height
		short signature[] = {0x72, 0x25, 0x3D, 0x58, 0x02, 0x00, 0x00};
		std::uint8_t replacement[] = {0x76, 0x25};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, 2));
	}

	if(!patches->install() || !locateWindowHandle()) {
		restore();
		throw HookException("Resolution patches failed!");
	}
}

bool locateWindowHandle() {
	short signature[] = {0x89, 0x35, -1, -1, -1, -1, 0xFF, 0x15, -1, -1, -1, -1, 0x3B, 0xC5};
	CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
	std::vector<std::uintptr_t> addresses = cf.find();
		
	if(addresses.empty()) {
		return false;
	}

	//@todo switch std::uintptr_t to std::uint8_t*
	std::uint8_t* addresstmp = reinterpret_cast<std::uint8_t*>(addresses[0]);
	addresstmp += 2;
	std::uint32_t* addresstmp2 = reinterpret_cast<std::uint32_t*>(addresstmp);
	window = reinterpret_cast<HWND*>(*addresstmp2);
	return true;
}

void restore() {
	patches.reset();
}

void setWindowed() {
	D3DVIEWPORT9 viewport;
	
	if(D3DHook::pDevice->GetViewport(&viewport) != D3D_OK) {
		hkDrawText("Failure!", C_TEXT_RED);
		return;
	}

	HMONITOR monitor = MonitorFromWindow(*window, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monInfo = { sizeof(MONITORINFO) };
		
	if(!GetMonitorInfo(monitor, &monInfo)) {
		hkDrawText("Failure!", C_TEXT_RED);
		return;
	}

	if(monInfo.rcMonitor.right - monInfo.rcMonitor.left != viewport.Width ||
		monInfo.rcMonitor.bottom - monInfo.rcMonitor.top != viewport.Height) {
		return;
	}

	LONG x = monInfo.rcMonitor.left;
	LONG y = monInfo.rcMonitor.top;
	LONG x2 = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
	LONG y2 = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
	
	LONG style = prevStyle;
	
	if(displayMode == WINDOWED) {
		prevStyle = GetWindowLong(*window, GWL_STYLE);
		style = WS_POPUP | WS_VISIBLE;
	}

	ShowWindow(*window, SW_HIDE);

	if(SetWindowLong(*window, GWL_STYLE, style) == 0) {
		hkDrawText("Failure!", C_TEXT_RED);
		return;
	}

	if(!SetWindowPos(*window, NULL, x, y, x2, y2, 0)) {
		hkDrawText("Failure!", C_TEXT_RED);
		return;
	}

	displayMode = displayMode == WINDOWED? WINDOWED_FULLSCREEN : WINDOWED;
}

void toggleDisplayMode() {
	setWindowed();
}

};