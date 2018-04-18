#include "GSRedirect.h"
#include "PatchGroup.h"
#include "Patcher.h"
#include "Codefinder.h"
#include <memory>
#include <cstdint>

namespace GSRedirect {

std::unique_ptr<PatchGroup> patches = nullptr;
char* master = "s1.master.hosthpc.com";
char* motd = "http://hpcup.bungie.net/motd/vercheck.asp?productid=%d&versionuniqueid=%s&distid=%d";

void install() {
	if(!patches) {
		patches.reset();
	}

	patches = std::unique_ptr<PatchGroup>(new PatchGroup());

	std::uint8_t replacement[sizeof(master)];
	memcpy(replacement, &master, sizeof(master));

	{ //%s.master.gamespy.com
		short signature[] = {0x68, -1, -1, -1, -1, 0x51, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x0C, 0x8D,
			0x5C, 0x24, 0x10, 0x68, 0xFC, 0x6C};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	{ //"%s.ms%d.gamespy.com"
		short signature[] = {0x68, -1, -1, -1, -1, 0x50, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x10, 0x68,
			0xEE, 0x70, 0x00, 0x00};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	memcpy(replacement, &motd, sizeof(motd));

	{ //motd
		short signature[] = {0x68, -1, -1, -1, -1, 0x68, -1, -1, -1, -1, 0x89, 0x3E, 0x89, 0x56, 0x04,
			0xE8, -1, -1, -1, -1, 0x56};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}
	
	//don't care if it fails, probably 1.10
	if(!patches->install()) {
		patches.reset();
	}
}

void uninstall() {
	patches.reset();
}

}