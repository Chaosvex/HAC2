#include "ObjectLimit.h"
#include "EngineTypes.h"
#include "Patcher.h"
#include "PatchGroup.h"
#include <memory>

namespace EngineLimits { namespace Objects {

namespace et = EngineTypes;

std::unique_ptr<PatchGroup> patches = nullptr;

void set(std::uint16_t limit) {
	patches = std::unique_ptr<PatchGroup>(new PatchGroup());
	std::uint8_t* newLimit = reinterpret_cast<std::uint8_t*>(&limit);

	{
		short signature[] = {0xB9, -1, -1, -1, -1, 0x2B, 0xCA, 0x68};
		std::uint8_t replacement[] = {0xB9, newLimit[0], newLimit[1], 0x00, 0x00};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	{
		short signature[] = {0x68, -1, -1, -1, -1, 0x68, -1, -1, -1, -1, 0x89, 0x0D, -1, -1, -1, -1, 0xC6, 0x40};
		std::uint8_t replacement[] = {0x68, newLimit[0], newLimit[1], 0x00, 0x00};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	if(!patches->install()) {
		restore();
		throw HookException("Unable to increase visible object limit!");
	}
}

void restore() {
	patches.reset();
}

}};