#include "ParticleLimits.h"
#include "EngineTypes.h"
#include "console.h"
#include "Patcher.h"
#include "PatchGroup.h"
#include <memory>

namespace EngineLimits { namespace Particles {

namespace et = EngineTypes;

std::unique_ptr<PatchGroup> patches = nullptr;
std::unique_ptr<et::RenderedParticle> particles = nullptr;

void set(std::uint16_t limit) {
	patches = std::unique_ptr<PatchGroup>(new PatchGroup());
	particles = std::unique_ptr<et::RenderedParticle>(new et::RenderedParticle[20000]);
	auto pParticles = particles.get(); 
	std::uint8_t newArray[sizeof(pParticles)];
	memcpy(newArray, &pParticles, sizeof(pParticles));

	{
		short signature[] = {0x68, 0x00, 0x04, 0x00, 0x00, 0x68, -1, -1, -1, -1, 0xBB, 0x70};
		std::uint8_t* newLimit = reinterpret_cast<std::uint8_t*>(&limit);
		std::uint8_t replacement[] = {0x68, newLimit[0], newLimit[1], 0x00, 0x00};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	{
		short signature[] = {0x8D, 0x8C, 0xCC, 0x00, 0x05, 0x00, 0x00};
		std::uint8_t replacement[] = {0x8D, 0x0C, 0xCD, newArray[0], newArray[1], newArray[2], newArray[3]};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));	
	}

	{
		short signature[] = {0x8D, 0x84, 0xC4, 0x04, 0x05, 0x00};
		std::uint8_t replacement[] = {0x8D, 0x04, 0xC5, newArray[0], newArray[1], newArray[2], newArray[3]};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	{
		short signature[] = {0x8D, 0x8C, 0x24, 0x04, 0x05, 0x00};
		std::uint8_t replacement[] = {0x8D, 0x0D, newArray[0], newArray[1], newArray[2], newArray[3], 0x90};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	{
		short signature[] = {0x8D, 0x84, 0x24, 0x04, 0x05, 0x00};
		std::uint8_t replacement[] = {0x8D, 0x05, newArray[0] + 4, newArray[1], newArray[2], newArray[3], 0x90};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	{
		short signature[] = {0x8D, 0x84, 0x24, 0x00, 0x05, 0x00};
		std::uint8_t replacement[] = {0x8D, 0x05, newArray[0], newArray[1], newArray[2], newArray[3], 0x90};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}
	
	if(!patches->install()) {
		restore();
	}
}

void restore() {
	std::uint16_t* lim = (std::uint16_t*)0x402BF97C;
	//If HAC buffer > 1024, restoring without resetting limit will crash.
	//If HAC buffer < 1024, resetting to 1024 before restoring buffer will crash.
	//Having said that, this function shouldn't be called after the game has been loaded.
	if(*lim > 1024) {
		*lim = 1024;
	}

	patches.reset();
	particles.reset();
	
	*lim = 1024;
}

}};