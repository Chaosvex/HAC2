#include "EngineFunctions.h"
#include "EngineState.h"
#include "EnginePointers.h"
#include <string>
#include <sstream>

std::uintptr_t HUDPrint, SoundPlay, ChatSend, GenMD5, Connect, CommandExecute, 
			   DisplayMenu, LoadMap, CommandInject, ChatLocalSend,
			   OverlayResolve, GetLocalPlayerIndex;
float* pDrawDistance;
char* VehicleControl;
std::int16_t* stringIndex;
bool* displayAttentionBox;

CRITICAL_SECTION critSection;

using namespace EngineTypes;

namespace {
	void RewindBuffer();
}

/* Critical sections are used to protect all engine functions as a precaution
   rather than assuming they're all thread safe. Won't help if HAC and Halo
   call the same non-thread safe function at the same time. */
void initCritSection() {
	InitializeCriticalSectionAndSpinCount(&critSection, 1024);
	serverJoinCritSection = CreateSemaphore(NULL, 1, 1, NULL); //temporary
}

void destroyCritSection() {
	DeleteCriticalSection(&critSection);
}

void HUDMessage(const std::wstring& message) {
	HUDMessage(message.c_str());
}

void HUDMessage(const std::string& message) {
	std::wstring wmessage;
	wmessage.assign(message.begin(), message.end());
	HUDMessage(wmessage.c_str());
}

void HUDMessage(const WCHAR* message) {
	wchar_t copy[64];
	wcscpy_s(copy, 64, message);

	EnterCriticalSection(&critSection);
	__asm {
		lea	eax, copy
		push eax
		mov	eax, 0 // player index
		call HUDPrint
		add	esp, 4
	}
	LeaveCriticalSection(&critSection);
}

void PlayMPSound(multiplayer_sound index) {
	__asm {
		push 0
		movsx esi, index
		or edi, -1
		call SoundPlay
		add	esp, 4
	}
}

/* TODO: Not thread safe - should send a copy of the buffer to be sent,
 * not the original as it may have been modified by the time the networking
 * thread requires it. Warning, memory leak!
 */
void ChatMessage(const WCHAR* message, CHAT_TYPE ctype) {
	wchar_t *copy = new wchar_t[64];
	wchar_t *copy2 = new wchar_t[64];
	wcscpy_s(copy, 64, message);
	wcscpy_s(copy2, 64, message);

	EnterCriticalSection(&critSection);

	__asm {
		mov eax, ctype
		push ebx
		mov edx, copy
		mov esi, copy2
		push esp
		call ChatSend
		add esp, 8
	}

	LeaveCriticalSection(&critSection);
}

void chatLocal(const std::string& message) {
	std::wstring wmessage;
	wmessage.assign(message.begin(), message.end());
	const wchar_t* msg = wmessage.c_str();

	__asm {
		push msg
		call ChatLocalSend
		add esp, 4
	}
}

void GenerateMD5(const char* data, DWORD data_length, char* output) {
	EnterCriticalSection(&critSection);

	__asm {
		push output
		push data_length
		push data
		call GenMD5
		add esp, 0x0C
	}

	LeaveCriticalSection(&critSection);
}

/*
 * Forces Halo to join a server - doesn't seem to have any
 * stability problems even when called from another thread.
 */
void serverConnect(const char *address, const wchar_t *password) {
	EnterCriticalSection(&critSection);

	_asm {
		mov edx, password
		mov eax, address
		push edx
		call Connect
		add esp, 4
	}

	LeaveCriticalSection(&critSection);
}

void commandConnect(const std::string& ip, const uint16_t& port, const std::wstring& password) {
	std::stringstream info;
	char wpassword[33];
	sprintf_s(wpassword, 32, "%ws", password.c_str());
	info << "connect " << ip << ":" << port << " " << wpassword;
	ShowMainMenuHaloThread();
	ExecuteCommand(info.str().c_str());
}

void DrawDistance(float distance) {
	*pDrawDistance = distance;
}

void ExecuteCommand(const char* command, bool rewind) {
	EnterCriticalSection(&critSection);

	__asm {
		mov edi, command
		push edi
		call CommandExecute
		add esp, 4
	}

	if(rewind) {
		RewindBuffer();
	}

	LeaveCriticalSection(&critSection);
}

void ShowMainMenu() {
	EnterCriticalSection(&critSection);

	__asm {
		call DisplayMenu
	}

	LeaveCriticalSection(&critSection);
}

//@todo - make this useful
void attentionBox(const std::string& message, bool quit) {
	*stringIndex = 21;
}

/* 
 * Causes Halo's main thread to load the main menu as opposed
 * to one of HAC's threads
 */
void ShowMainMenuHaloThread() {
	*stringIndex = -1;
	*displayAttentionBox = true;
}


/*
 *  Calls Halo's map load function to add a map to the table.
 *  Checksum needs to be added to the hash-map to bypass the game's
 *  own checksum function or it'll crash (after checksumming)!
 */
void addMap(const char* name) {
	__asm {
		pushfd
		pushad
		push 0x13
		mov eax, name
		call LoadMap
		add esp, 4
		popad
		popfd
	}
}

void ToggleMTV(bool state) {
	*VehicleControl = state? 0xEB : 0x74; //jmp : je
}

namespace {

/* 
 * Rewind the console history buffer back by one command and zero it out.
 * This is to allow HAC to execute commands without the user seeing them
 * in their history. Wipes out their oldest command, an acceptable
 * compromise for not having to implement more code for the same effect.
 */
void RewindBuffer() {
	void *location = (*histBuffLen * 255) + histBuff;
	memset(location, 0, 255);
	*histBuffLen = *histBuffLen == 0? 7 : *histBuffLen - 1;
}

}