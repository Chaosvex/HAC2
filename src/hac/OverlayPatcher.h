#pragma once

#include <vector>
#include <memory>

class PatchGroup;

class OverlayPatcher {
public:
	OverlayPatcher() { }
	~OverlayPatcher();
	void patch();

private:
	std::vector<std::unique_ptr<PatchGroup>> hooks;
	OverlayPatcher(const OverlayPatcher&);
};