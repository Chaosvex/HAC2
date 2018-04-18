#include "ConsoleCommands.h"
#include "CustomChat.h"
#include "ArgContainer.h"
#include "console.h"
#include "TimerManager.h"
#include "EngineTypes.h"
#include "EngineFunctions.h"
#include "InitHooks.h"
#include "DebugHelper.h"
#include "Bookmarker.h"
#include "mapCache.h"
#include "FoVControl.h"
#include "BSPPatcher.h"
#include "BudgetDisplay.h"
#include "EventSubscriber.h"
#include "MapControl.h"
#include "DisplayUpdater.h"
#include "SlotQueue.h"
#include "EnginePointers.h"
#include "Shared.h"
#include "misc.h"
#include "HUDScale.h"
#include "Preferences.h"
#include "Language.h"
#include "Optic.h"
#include "MapChecksum.h"
#include "ScopeBlur.h"
#include "ParticleLimits.h"
#include "ObjectLimit.h"
#include "DisplayModes.h"
#include "MapDownload.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <process.h>
#include <sstream>

#include "SettingsUI.h" //@todo remove

void execFlushCache(); //todo move to map management code
void execBypass();
void  __cdecl printEgg(void*);
std::uintptr_t ProcessCommands;
short* histBuffLen;
char* histBuff;
void bufferCommand(char* input);

#include <vector>
#include "EnginePointers.h"
#include "EngineTypes.h"
#include "DisplayModes.h"
using namespace EngineTypes;
using std::vector;
template<typename T>
vector<T*> tagSearch(uint32_t tagClass, const char* tagName = NULL) {
	TagEntry* table = pTagTableHeader->TagTableEntryPointer;
	vector<T*> tags;

	for(int i = 0; i < pTagTableHeader->TagCount; i++) {
		if(table[i].class0 == tagClass) {
			if(tagName == NULL || _stricmp(table[i].tagNameAddress, tagName) == 0) {
				tags.emplace_back(reinterpret_cast<T*>(table[i].tagStruct));
			}
		}
	}

	return tags;
}

/*
 * All console commands entered by the player flow through this function.
 * If the command matches one of HAC's, handle it and alter the command
 * buffer to prevent Halo from displaying a 'command not found' error.
 */
void ConsoleRead(char* input) {
	std::string command(input);
	ArgContainer args(command);
	bool processed = true;

	if(args[0].compare("login") == 0) {
		if(args.argc > 2) {
			//char md5[33];
			//GenerateMD5(passwordBuff, 1, md5);
			//shared_ptr<Event> event(new LoginEvent(args[1], md5));
			//dispatcher->enqueue(event);
		} else {
			hkDrawText("Usage: login <username> <password>", C_TEXT_YELLOW);
		}
	} else if(args[0].compare("rasterizer_wireframe") == 0) {
		//swallow the command 
	} else if(args[0].compare("budget_show") == 0) {
		if(args.argc > 1) {
			if(args.ProcessBoolValue()) {
				displayBudgetStart();
			} else {
				displayBudgetStop();
			}
		}
	} else if(args[0].compare("fov") == 0) {
		if(args.argc > 1) {
			FoV::set(static_cast<float>(atof(args[1].c_str())));
		}
		Preferences::add("fov", FoV::get());
	} else if(args[0].compare("draw") == 0) {
		if(args.argc > 1) {
			DrawDistance(static_cast<float>(atof(args[1].c_str())));
		}
	}  else if(args[0].compare("play_during_queue") == 0) {
		if(args.argc > 1) {
			Preferences::add("play_during_queue", args.ProcessBoolValue());
		}
	} else if(args[0].compare("devmode") == 0) {
		if(args.argc > 1) {
			if(args.ProcessBoolValue()) {
				hkDrawText(Language::GetString(DEVMODE_ENABLE), C_TEXT_YELLOW);
			} else {
				hkDrawText(Language::GetString(DEVMODE_DISABLE), C_TEXT_YELLOW);
			}
		} else {
			hkDrawText("Usage: devmode <true/false>", C_TEXT_BLUE);
		}
	} else if(args[0].compare("copy") == 0) {
		if(args.argc > 1) {
			Bookmarker bookmark;
			if(bookmark.clipboard(atoi(args[1].c_str()))) {
				hkDrawText(Language::GetString(BOOKMARK_COPIED), C_TEXT_GREEN);
			} else {
				hkDrawText(Language::GetString(BOOKMARK_LOCATE_ERROR), C_TEXT_RED);
			}
		} else {
			std::wstringstream stream;
			stream << "connect " << serverAddress << " " << reconnectPassword;
			std::wstring copy = stream.str();
			HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, (copy.length() + 1) * sizeof(wchar_t));
			if(hMem != NULL) {
				memcpy(GlobalLock(hMem), copy.c_str(), (copy.length() + 1) * sizeof(wchar_t));
				GlobalUnlock(hMem);
				OpenClipboard(0);
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, hMem);
				CloseClipboard();
				hkDrawText(Language::GetString(SERVER_COPIED), C_TEXT_GREEN);
			} else {
				hkDrawText(Language::GetString(SERVER_COPY_ERROR), C_TEXT_RED);
			}
		}
	} else if(args[0].compare("hac_flush_cache") == 0) {
		execFlushCache();
	} else if(args[0].compare("panic") == 0) {
		*overlayControl = 0;
	} else if(args[0].compare("hac_debug_bypass") == 0) {
		execBypass();
	//#ifdef _DEBUG
	} else if(args[0].compare("timer_enable") == 0) {
		enableTimers(false);
	} else if(args[0].compare("timer_disable") == 0) {
		disableTimers();
	} else if(args[0].compare("timer_reset") == 0) {
		resetTimer();
	} else if(args[0].compare("timer_stop") == 0) {
		endTimer(false);
	} else if(args[0].compare("timer_start") == 0) {
		beginTimerUser();
	} else if(args[0].compare("play") == 0) {
		PlayMPSound((EngineTypes::multiplayer_sound)atoi(args[1].c_str()));
	} else if(args[0].compare("timer_broadcast") == 0) {
		if(args.argc >= 2) {
			bool enable = args.ProcessBoolValue();
			timerBroadcast(enable);
			if(enable) {
				hkDrawText(Language::GetString(TIMER_BC_ENABLE), C_TEXT_YELLOW);
			} else {
				hkDrawText(Language::GetString(TIMER_BC_DISABLE), C_TEXT_YELLOW);
			}
		} else {
			hkDrawText("Usage: timer_broadcast <true/false>", C_TEXT_BLUE);
		}
	//#endif
	} else if(args[0].compare("hac_unload") == 0) {
#ifdef _DEBUG
		endTimer(true);
#endif
		_beginthread(DebugHelper::UnloadHAC, NULL, NULL);
	} else if(args[0].compare("hac_reload") == 0) {
		hkDrawText(Language::GetString(HAC_RELOAD), C_TEXT_YELLOW);
#ifdef _DEBUG
		endTimer(true);
#endif
		DebugHelper::ReloadHAC();
	} else if(args[0].compare("recalculate") == 0) {
		if(args.argc > 1) {
			try {
				std::string path = getWorkingDir() + "maps/" + args[1] + ".map";
				MapChecksum checksummer(path);
				std::uint32_t checksum = checksummer.checksum();
				addChecksum(args[1], checksum);
				hkDrawText(Language::GetString(CHECKSUM_RC_SUCCESS), C_TEXT_GREEN);
			} catch(std::runtime_error& e) {
				hkDrawText(e.what(), C_TEXT_RED);
			}
		} else {
			hkDrawText("Usage: recalculate \"map name\"", C_TEXT_YELLOW);
		}
	} else if(args[0].compare("about") == 0) {
		char buffer[128];
		sprintf_s(buffer, 128, Language::GetString(HAC_VERSION).c_str(), "2.x.x.0a (13/10/16)", "http://blog.haloanticheat.com");
		hkDrawText(buffer, C_TEXT_GREEN);
	} else if(args[0].compare("devcam") == 0) {
		if(args.argc > 1) {
			if(args.ProcessBoolValue()) {
				ExecuteCommand("debug_camera_save");
				ExecuteCommand("debug_camera_load");
				HUDMessage("Hold the mouse-wheel down to control the camera");
			} else {
				ExecuteCommand("camera_control 0");
			}
		} else {
			hkDrawText("Usage: devcam <0/1>", C_TEXT_YELLOW);
		}
	} else if(args[0].compare("hac_widescreen") == 0) {
		if(args.argc > 1) {
			bool preference = args.ProcessBoolValue();

			if(preference) {
				hkDrawText("HUD fix enabled.", C_TEXT_GREEN);
				HUDScale::ratioFix(true);
			} else {
				hkDrawText("HUD fix disabled", C_TEXT_GREEN);
				HUDScale::reset();
			}

			Preferences::add("hud_ratio_fix", preference);
		}
	} else if(args[0].compare("camo_fix") == 0) {
		if(args.argc > 1) {
			bool preference = args.ProcessBoolValue();

			if(preference) {
				hkDrawText("Active camo fix enabled.", C_TEXT_GREEN);
				*activeCamoDisable = false;
			} else {
				hkDrawText("Active camo fix disabled", C_TEXT_GREEN);
				*activeCamoDisable = true;
			}

			Preferences::add("active_camo_fix", preference);
		}
	} else if(args[0].compare("scope_blur") == 0) {
		if(args.argc > 1) {
			bool preference = args.ProcessBoolValue();

			if(preference) {
				ScopeBlurFix::disable();
				hkDrawText("Scope blur disabled!", C_TEXT_GREEN);
			} else {
				ScopeBlurFix::enable();
				hkDrawText("Scope blur enabled!", C_TEXT_GREEN);
			}

			Preferences::add("no_scope_blur", !preference);
		}
	} else if(args[0].compare("visible_objects") == 0) {
		if(args.argc > 1) {
			std::uint16_t objects = atoi(args[1].c_str());
			EngineLimits::Objects::set(objects);
			Preferences::add("visible_objects", objects);
			hkDrawText("Visible object limit set.", C_TEXT_GREEN);
		}
	} else if(args[0].compare("particles") == 0) {
		if(args.argc > 1) {
			std::uint16_t particles = atoi(args[1].c_str());
			EngineLimits::Particles::set(particles);
			Preferences::add("particles", particles);
		}
	} else if(args[0].compare("optic") == 0) {
		if(args.argc == 1) {
			Optic::list();
		} else if(args.argc == 2) {
			if(args[1] == "unload") {
				Optic::unload();
			}
		} else if(args.argc == 3) {
			if(args[1] == "load") {
				Optic::load(args[2]);
				Preferences::add("optic_pack", args[2]);
			}
		} else {
			hkDrawText("Usage: optic <pack name>", C_TEXT_RED);
		}
	} else if(args[0].compare("custom_chat") == 0) {
		if(args.argc == 1) {
			return;
		}

		bool enable = args.ProcessBoolValue();

		if(enable) {
			hkDrawText("Custom chat enabled", C_TEXT_GREEN);
			Chat::enable();
		} else {
			hkDrawText("Custom chat disabled", C_TEXT_GREEN);
			Chat::disable();
		}

		Preferences::add("custom_chat", enable);
	} else if(args[0].compare("redirect_kill_feed") == 0) {
		if(args.argc == 1) {
			return;
		}

		bool redirect = args.ProcessBoolValue();

		if(redirect) {
			hkDrawText("Redirecting kill feed to HUD", C_TEXT_GREEN);
		} else {
			hkDrawText("Redirecting disabled", C_TEXT_GREEN);
		}

		Preferences::add("redirect_kill_feed", redirect);
	} else if(args[0].compare("font_size") == 0) {
		if(args.argc > 1) {
			try {
				unsigned int size = boost::lexical_cast<unsigned int>(args[1]);
				Preferences::add("font_size", size);
				Chat::resolutionChange();
				hkDrawText("Font size set.", C_TEXT_GREEN);
			} catch(boost::bad_lexical_cast&) {
				hkDrawText("Invalid font size.", C_TEXT_RED);
			}
		} else {
			hkDrawText("Usage: font size <size>", C_TEXT_YELLOW);
		}
	} else if(args[0].compare("time") == 0) {
		if(args.argc > 1) {
			*tickrate = boost::lexical_cast<float>(args[1]);
		}
	} else if(args[0].compare("map_download") == 0) {
		if(args.argc == 2) {
			std::string* map = new std::string(std::string("mapname\\") + args[1]); // wat!
			beginDownload(map->c_str());
		} else {
			hkDrawText("Usage: map_download <name>", C_TEXT_YELLOW);
		}
	} else {
		processed = false;
	}

	if(processed) {
		bufferCommand(input);
		input[0] = 0x3B; //prevent hooked function from processing
	}
}

void bufferCommand(char* input) {
	//Command buffer is a fixed array, 8*255 bytes
	*histBuffLen = (*histBuffLen + 1) % 8;
	char *location = (*histBuffLen * 255) + histBuff;
	strcpy_s(location, 255, input);
}

void execFlushCache() {
	if(flushCache()) {
		hkDrawText(Language::GetString(CACHE_FLUSH_SUCCESS), C_TEXT_GREEN);
	} else {
		hkDrawText(Language::GetString(CACHE_FLUSH_ERROR), C_TEXT_RED);
	}
}

void execBypass() {
	_beginthread(printEgg, 0, NULL);
}

/*
 * Codecave for redirecting console input.
 */
void _declspec(naked) ConsoleReadCC() {
	__asm {
		pushfd
		pushad
		push edi
		call ConsoleRead
		add esp, 4
		popad
		popfd

		//Original
		mov al, [edi]
		sub esp, 0x00000500
		jmp ProcessCommands
	}
}