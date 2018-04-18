#include "PatchGroup.h"
#include "IHook.h"

void PatchGroup::add(IHook* hook) {
	hooks.push_back(hook);
}

bool PatchGroup::install() {
	for(std::vector<int>::size_type i = 0; i != hooks.size(); i++) {
		if(!hooks[i]->Install()) {
			return false;
		}
	}
	return true;
}

void PatchGroup::uninstall() {
	for(std::vector<int>::size_type i = 0; i != hooks.size(); i++) {
		delete hooks[i];
	}
}

PatchGroup::~PatchGroup() {
	uninstall();
}