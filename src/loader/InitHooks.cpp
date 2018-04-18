#include "InitHooks.h"
#include "Shared.h"

PatchGroup* loadHook();
void installHooks();
void uninstallHooks();
vector <PatchGroup*> hooks;

void InitialiseHooks() {	
	try {
		installHooks();
	} catch(HookException) {
		UninstallHooks();
		throw;
	}
}

void installHooks() {
	hooks.push_back(loadHook());
}

void UninstallHooks() {
	for(std::vector<int>::size_type i = 0; i != hooks.size(); i++) {
		delete hooks[i];
	}
	hooks.clear();
}

PatchGroup* loadHook() {
	char* signature = "8B 43 20 83 C4 04 85 C0 0F 84 76 00 00 00 68";
	PatchGroup *group = new PatchGroup();
	group->add(new CaveHook(signature, 6, (DWORD)LaunchBlockCave, &ContinueLaunch));

	if(!group->install()) {
		delete group;
		throw HookException("Launch hook failed!");
	}
	
	return group;
}