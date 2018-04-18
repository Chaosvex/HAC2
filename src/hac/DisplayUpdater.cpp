#include "DisplayUpdater.h"
#include "UIControl.h"
#include "DebugHelper.h"
#include "PatchGroup.h"
#include "console.h"
#include "OverlayPatcher.h"
#include "PatchGroup.h"
#include "EngineTypes.h"
#include <Windows.h>
#include <sstream>
#include <string>

using namespace EngineTypes;

DWORD* overlayControl;
HANDLE mainThread;
CRITICAL_SECTION DisplayUpdater::csection;
const wchar_t* DisplayUpdater::format = L"Downloading %hs - %.1fMB/%.1fMB, %dKB/s";
DisplayUpdater* DisplayUpdater::instance;

DisplayUpdater::DisplayUpdater(const std::string& map)
	: overlayPatch(new OverlayPatcher), map(map), location(NULL), message(new WCHAR[256]()) {
	initialise();
}

DisplayUpdater::DisplayUpdater()
	: overlayPatch(new OverlayPatcher), map(""), location(NULL), message(new WCHAR[256]()) {
	initialise();
}

void DisplayUpdater::initialise() {
	InitializeCriticalSection(&csection);
	restore = true;
	oldOverlay = NO_OVERLAY;
	getLocation();
	instance = this;

	try {
		overlayPatch->patch();
	} catch(std::runtime_error) {
		hkDrawText("An error occured while patching the overlay!", C_TEXT_RED);
	}
}

void DisplayUpdater::updateProgress(float done, float remaining, int speed) {
	EnterCriticalSection(&csection);
	swprintf_s(message, 256, format, map.c_str(), done, remaining, speed);
	LeaveCriticalSection(&csection);
}

void DisplayUpdater::showMessage(const WCHAR* message) const {
	EnterCriticalSection(&csection);
	wcscpy_s(this->message, 256, message);
	LeaveCriticalSection(&csection);
}

void DisplayUpdater::showMessage(const std::string& message) const {
	EnterCriticalSection(&csection);
	std::wstringstream wmsg;
	wmsg << message.c_str();
	wcscpy_s(this->message, 256, wmsg.str().c_str());
	LeaveCriticalSection(&csection);
}

void DisplayUpdater::begin() const {
	oldOverlay = *overlayControl;
	*overlayControl = CONNECTING_TO_SERVER;
}

void DisplayUpdater::unpatch() {
	delete overlayPatch;
	overlayPatch = NULL;
}

/*
 * House of hacks
 */
void DisplayUpdater::getLocation() {
	void* pLocation = stringEntry(CONNECTING_TO_SERVER - 1);
	bool stateChange = false;

	/* TEMP FIX TO RESTORE OLD LOCATION */
	if(location != NULL && pLocation != location) {
		memcpy_s(location, sizeof(DisplayString), &original, sizeof(DisplayString));
		stateChange = true;
	}

	if(location != pLocation) {
		stateChange = true;
	}

	location = pLocation;
	/* END */

	if(location == NULL) {
		hkDrawText("Error parsing tag structures!", C_TEXT_RED);
		return;
	}

	//Back original info up
	if(stateChange) {
		memcpy_s(&original, sizeof(DisplayString), location, sizeof(DisplayString));
	}

	//Update string pointer and length...=(
	uint32_t *length = &(static_cast<DisplayString*>(location))->length;
	*length = 64 * sizeof(WCHAR); //Halo requires size in bytes
	WCHAR** pMessage = &(static_cast<DisplayString*>(location))->message;
	*pMessage = message;
}

DisplayUpdater::~DisplayUpdater() {
	instance = NULL;

	if(location != NULL) {
		memcpy_s(location, sizeof(DisplayString), &original, sizeof(DisplayString));
	}

	if(restore) {
		*overlayControl = oldOverlay;
	}

	DeleteCriticalSection(&csection);
	delete[] message;
	delete overlayPatch;
}

void DisplayUpdater::notify() {
	if(instance != NULL) {
		instance->getLocation();
	}
}