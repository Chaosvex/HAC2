#include "networking.h"
#include "console.h"
#include "EngineFunctions.h"
#include "EnginePointers.h"
#include "InitHooks.h"
#include "resource.h"
#include "versionchanger.h"
#include "CustomChat.h"
#include "Optic.h"
#include "EventDispatcher.h"
#include "PacketTranslator.h"
#include "CustomChat.h"
#include "Preferences.h"
#include "FoVControl.h"
#include "DebugHelper.h"
#include "Events.h"
#include "Shared.h"
#include "CrashHandler.h"
#include "Language.h"
#include "ScopeBlur.h"
#include "ParticleLimits.h"
#include "misc.h"
#include "MapControl.h"
#include "ObjectLimit.h"
#include "DiscordHandlers.h"
#include <discord-rpc.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>
#include <string>
#include <memory>
#include <vector>

#define ThreadHideFromDebugger 0x11
typedef NTSTATUS (NTAPI *pNtSetInformationThread) (HANDLE, UINT, PVOID, ULONG);
typedef BOOL (WINAPI *pSetProcessDEPPolicy) (DWORD);

EventDispatcher* dispatcher;
LPTOP_LEVEL_EXCEPTION_FILTER oFilter;
std::vector<EventSubscriber*> modules;
std::vector<HANDLE> threads;
void onQuit(void* args), onLaunch(void* args), unload();
unsigned int __stdcall hacLaunch(void*);
bool terminateSubscribers();
void loadPreferences();
void initDiscord();

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
	if(reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(module);

		HANDLE initialiser = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, hacLaunch, 0, CREATE_SUSPENDED, 0));
		
		if(initialiser == NULL) {
			OutputDebugString("Thread failure.");
			return FALSE;
		}

#ifndef _DEBUG
		NTSTATUS status;
		HMODULE mod = GetModuleHandle("ntdll.dll");
			
		if(mod == NULL) {
			return FALSE;
		}

		pNtSetInformationThread ntSetInfo = (pNtSetInformationThread)GetProcAddress(mod, "NtSetInformationThread");
		status = ntSetInfo(initialiser, ThreadHideFromDebugger, NULL, 0);

		if(status != 0) {
			return FALSE;
		}
#endif

		try {
			HookManager::install();
			HookManager::launchCallback(onLaunch, initialiser);
			HookManager::quitCallback(onQuit);
		} catch(HookException& e) {
			HookManager::uninstall();
			MessageBoxA(NULL, e.what(), Language::GetString(WORD_ERROR).c_str(), MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		ResumeThread(initialiser);
	}

	if(reason == DLL_PROCESS_DETACH) {
		unload();
	}
    
	return TRUE;
}

/*  
 * Only called for a graceful game quit. Don't really want to
 *  save preferences in the event of a crash.
 */
void onQuit(void*) {
	Preferences::save();
	Chat::disable();
	Optic::unload();
    Discord_Shutdown();
}

void unload() {
	HookManager::uninstall();
	terminateSubscribers();
	SetUnhandledExceptionFilter(oFilter);
}

/*
 * Blocks Halo's main thread until HAC's initialisation
 * has completed. Only occurs at game launch, not reload.
 */
void onLaunch(void* args) {
	initCritSection();

	CreateDirectory(std::string(getWorkingDir() + "\\maps").c_str(), NULL);

	if(IsProcessElevated()) {
		copyMaps();
	}

	//Install exception handler inside Halo's main thread
	oFilter = SetUnhandledExceptionFilter(crashHandler);

	//Wait for the init thread to complete
	HANDLE initialiser = static_cast<HANDLE>(args);
	DWORD result = WaitForSingleObject(initialiser, 10000);
	
	if(FAILED(result)) {
		if(result == WAIT_TIMEOUT) {
			OutputDebugString("Init wait timed out.");
		} else if(result == WAIT_FAILED) {
			DebugHelper::Translate("Thread waiting");
		}
	}
	
	DWORD exitCode;

	if(GetExitCodeThread(initialiser, &exitCode) == FALSE) {
		OutputDebugString("Couldn't retrieve exit code.");
		exitCode = -2;
	}

	if(exitCode != 0) {
		switch(exitCode) {
			case NET_CONNECT_FAIL:
				hkDrawText(Language::GetString(HAC_CONNECT_ERROR), C_TEXT_YELLOW);
				break;
			case STILL_ACTIVE:
				OutputDebugString("Init thread still active!");
			case -2 :
			default :
				hkDrawText(Language::GetString(HAC_START_ERROR), C_TEXT_RED);
				terminateSubscribers();
		}
	}

	CloseHandle(initialiser);
}

/*
 * Entry point for HAC's initialisation thread.
 * Starts the subsystems required for HAC to function.
 */
unsigned int __stdcall hacLaunch(void*) {
	SetUnhandledExceptionFilter(crashHandler);
	loadPreferences();
    initDiscord();

	//Start event dispatcher
	dispatcher = new EventDispatcher();
	
	//Start networking
	/*Networking *connection = new Networking();
	connection->registerDispatcher(modules);
	modules.emplace_back(connection);

	DWORD status = connection->init();
	if(status == NET_WINSOCK_FAIL || status == NET_RESOLVE_FAIL) {
		delete connection;
		return status;
	}*/

	DWORD status = NET_SUCCESS; //temporary

	//Start packet handler
	PacketTranslator *translator = new PacketTranslator();	
	translator->registerDispatcher(dispatcher);
	modules.emplace_back(translator);

	//Start anticheat
#ifdef ANTICHEAT
	Sentinel* sentinel = new Sentinel();
	sentinel->registerDispatcher(dispatcher);
	modules.emplace_back(sentinel);
	Anticheat* anticheat = new Anticheat();
	anticheat->registerDispatcher(dispatcher);
	modules.emplace_back(anticheat);
#endif

	dispatcher->registerSubscribers(modules); //Register all subscribers with dispatcher - @todo, invert
	modules.emplace_back(dispatcher);

	//Start all threads
	for(size_t i = 0; i < modules.size(); i++) {
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, NULL, modules[i]->start, (void*)modules[i], NULL, NULL);
		if(hThread != NULL) {
			threads.emplace_back(hThread);
		} else {
			DebugHelper::Translate("Thread creation");
			return -1;
		}
	}
	
	/* If the server is up and we connected, the ret code is 0 otherwise
	 * it's NET_CONNECT_FAIL. However, this isn't considered a failure,
	 * merely a prompt to tell the user that services will be unavailable
	 * until a connection is established. The only problem is if WinSock/threads
	 * failed to start correctly. */
	return status;
}

void loadPreferences() {
	Preferences::initialise();
	FoV::set(Preferences::find("fov", 70.0f));
	DrawDistance(Preferences::find("draw_distance", 1024.0f));
	ScopeBlurFix::set(Preferences::find("no_scope_blur", false));
	//EngineLimits::Particles::set(Preferences::find("particles", 1024));
	EngineLimits::Objects::set(Preferences::find("visible_objects", 1024));
	*activeCamoDisable = !Preferences::find("active_camo_fix", true);
}

void initDiscord() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = DiscordHandlers::ready;
    handlers.errored = DiscordHandlers::error;
    handlers.disconnected = DiscordHandlers::disconnected;
    handlers.joinGame = DiscordHandlers::join;
    handlers.spectateGame = DiscordHandlers::spectate;
    handlers.joinRequest = DiscordHandlers::joinRequest;
    Discord_Initialize(DISCORD_APP_ID, &handlers, 1, nullptr);
}

/*
 * Shuts down subscriber threads and frees memory.
 * Consecutive calls to this function should be safe as a
 * manual call may be made even after HAC has already
 * automatically cleaned up after a failure.
 */
bool terminateSubscribers() {
	bool success = true;
	dispatcher->enqueue(std::make_shared<Event>(THREAD_EXIT));

	modules.clear();

	if(threads.empty()) {
		return true;
	}

	//Wait for subscribers to exit gracefully
	DWORD result = WaitForMultipleObjects(threads.size(), &threads.front(), TRUE, 15000);
	
	if(result != WAIT_OBJECT_0) {
		success = false;
		if(result == WAIT_TIMEOUT) {
			OutputDebugString("Thread wait timed out.");
		} else if(result == WAIT_FAILED) {
			OutputDebugString("Wait failed.");
		}
	}

	//Check return values of threads
	for(auto i = threads.begin(); i != threads.end(); i++) {
		DWORD code = 0;
		if(GetExitCodeThread(*i, &code) != 0) {
			if(code == STILL_ACTIVE) {
				hkDrawText("Active thread found. Unloading may crash.", C_TEXT_RED);
				success = false;
			} else if(code != 0) {
				OutputDebugString("Thread did not exit cleanly.");
				success = false;
			}
		} else {
			OutputDebugString("Unable to fetch exit code.");
		}

		CloseHandle(*i);
	}

	threads.clear();

	return success;
}