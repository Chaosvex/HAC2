#include "Optic.h"
#include "OpticPack.h"
#include "OpticLua.h"
#include "OpticAPI.h"
#include "misc.h"
#include "console.h"
#include "OpticEventHandler.h"
#include "OpticHooks.h"
#include "OpticMisc.h"
#include "EventDispatcher.h"
#include "Shared.h"
#include <sstream>
#include <shlobj.h>
#include <Windows.h>
#include <memory>
#include <process.h>

namespace Optic {

std::unique_ptr<OpticEventHandler> handler(nullptr);

void render() {
	if(handler != nullptr) {
		handler->render();
	}
}

void load(const std::string& name) {
	if(handler != nullptr) {
		unload();
	}

	try {
		std::string zipName(name + ".zip");
		std::unique_ptr<OpticPack> pack(new OpticPack(zipName));
		std::unique_ptr<OpticEventHandler> listener(new OpticEventHandler(std::move(pack), dispatcher));
		handler = std::move(listener);
		HANDLE handle = (HANDLE)_beginthreadex(NULL, NULL, handler->start, static_cast<void*>(handler.get()), NULL, NULL);
		CloseHandle(handle);
		hkDrawText("Optic pack loaded!", C_TEXT_GREEN);
	} catch(OpticException& e) {
		hkDrawText(e.what(), C_TEXT_RED);
	}
}

void unload() {
	if(handler != nullptr) {
		handler->enqueue(std::make_shared<Event>(THREAD_EXIT));
		handler->wait();
		handler.reset();
		Optic::Hooks::uninstall();
		hkDrawText("Optic pack unloaded!", C_TEXT_GREEN);
	}
}

void reset() {
	if(handler != nullptr) {
		handler->reset();
	}
}

void lost() {
	if(handler != nullptr) {
		handler->lost();
	}
}

void list() {
	std::string expression = getWorkingDir() + "packs\\*.zip";
	WIN32_FIND_DATA FindFileData;
	HANDLE search = FindFirstFile(expression.c_str(), &FindFileData);

	if(search != INVALID_HANDLE_VALUE) {
		do {
			std::stringstream ss;
			try {
				OpticPackMeta meta = OpticPack::meta(FindFileData.cFileName);
				ss << meta.name << " by " << meta.author << " - " << meta.description;
				C_TEXT_COLOUR colour = C_TEXT_GREEN;

				Version version = {1, 0, 0};
				int status = versionCheck(meta.apiVersion, version);

				if(status == VERSION_RELATION::TENTATIVE_OKAY) {
					ss << " (out of date)";
					colour = C_TEXT_YELLOW;
				} else if(status == VERSION_RELATION::VERSION_IN_FUTURE) {
					ss << " (version in future?)";
					colour = C_TEXT_RED;
				} else if(status == VERSION_RELATION::UNKNOWN_VERSION) {
					ss << " (invalid version)";
					colour = C_TEXT_RED;
				} else if(status == VERSION_RELATION::OUT_OF_DATE) {
					ss << " (out of date)";
					colour = C_TEXT_RED;
				}
				
				hkDrawText(ss.str(), colour);
			} catch(OpticPackException& e) {
				ss << FindFileData.cFileName << " - " << e.what();
				hkDrawText(ss.str(), C_TEXT_RED);
			}
		} while(FindNextFile(search, &FindFileData) != 0);

		FindClose(search);
	} else {
		hkDrawText("An error occured while attempting to read packs.", C_TEXT_RED);
	}
}

};