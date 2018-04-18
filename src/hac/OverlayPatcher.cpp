#include "OverlayPatcher.h"
#include "Patcher.h"
#include "PatchGroup.h"
#include <memory>

const BYTE NOP = 0x90;

void OverlayPatcher::patch() {
	std::unique_ptr<PatchGroup> group = std::unique_ptr<PatchGroup>(new PatchGroup());

	BYTE replacement[] = {NOP, NOP, NOP, NOP, NOP, NOP};
	short signature1[] = {0x89, 0x1D, -1, -1, -1, -1, 0x89, 0x1D, -1, -1, -1, -1, 0x66, 0x89, 0x1D, -1, -1, -1, -1, 0x66, 0x89, 0x1D, -1, -1, -1, -1, 0x89, 0x35, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x88, 0x1D, -1, -1, -1, -1, 0x89, 0x35};
	short signature2[] = {0x89, 0x1D, -1, -1, -1, -1, 0x89, 0x1D, -1, -1, -1, -1, 0x66, 0x89, 0x1D, -1, -1, -1, -1, 0x66, 0x89, 0x1D, -1, -1, -1, -1, 0x89, 0x35, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0xB8, -1, -1, -1, -1, 0x8D, 0x50, 0x01};
	
	group->add(new PatchHook(signature1, sizeof(signature1) / 2, 0, replacement, 6));
	group->add(new PatchHook(signature2, sizeof(signature2) / 2, 0, replacement, 6));

	if(!group->install()) {
		throw std::runtime_error("Overlay patching failed!");
	}

	hooks.emplace_back(std::move(group));
}

OverlayPatcher::~OverlayPatcher() {
	hooks.clear();
}