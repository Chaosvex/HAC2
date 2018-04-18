#include "BSPPatcher.h"
#include "PatchGroup.h"
#include "Patcher.h"
#include "Codefinder.h"
#include <cstdint>
#include "DebugHelper.h"

namespace EngineLimits { namespace BSP {

std::unique_ptr<PatchGroup> patches = nullptr;
std::unique_ptr<std::uint8_t> buffer = nullptr;

const int DEFAULT_LIMIT = 16384;
const int NEW_LIMIT = DEFAULT_LIMIT * 4;
const int TEXTURE_D_SIZE = 4;
const int COORD_SIZE = 6;

void updateBufferReferences();
void increaseLimits();

void patch() {
	patches = std::unique_ptr<PatchGroup>(new PatchGroup());
	updateBufferReferences();
	increaseLimits();

	if(!patches->install()) {
		restore();
		throw HookException("BSP patches failed!");
	}
}

void restore() {
	patches.reset();
	buffer.reset();
}

void updateBufferReferences() {
	buffer = std::unique_ptr<std::uint8_t>(new std::uint8_t[NEW_LIMIT * TEXTURE_D_SIZE]);

	auto pBuffer = buffer.get(); 
	std::uint8_t replacement[sizeof(pBuffer)];
	memcpy(replacement, &pBuffer, sizeof(pBuffer));
	
	{ //buffer write - 0x00556240
		short signature[] = {0x68, -1, -1, -1, -1, 0xBB, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x04, 0x83, 0xF8,
							 0xFF, 0xA3};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	{ //texture - 0x0050FE3E 
		short signature[] = {0xB9, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x14, 0xE8, -1, -1, -1, -1, 0x66,
							 0x39, 0x1D};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	{ //depth - 0x005100AE
		short signature[] = {0xB9, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x14, 0xA1};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	{ //coords - 0x0055637C
		short signature[] = {0xB9, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0xA1, -1, -1, -1, -1, 0x83, 0xC4, 0x14};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	{ //shader - 0x00510073
		short signature[] = {0xB9, -1, -1, -1, -1, 0x89, 0x1D, -1, -1, -1, -1, 0x88, 0x1D};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	{ //0x00556546, 0x00556466
		short signature[] = {0xBB, -1, -1, -1, -1, 0x83, 0xFF, 0xFF, 0x74, -1, 0x8B, 0x84, 0x24, -1, -1, -1, -1, 0xE8,
							 -1, -1, -1, -1, 0x6A, 0x00, 0x6A, 0x00};
		CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
		for(auto i = cf.begin(); i != cf.end(); i++) {
			patches->add(new PatchHook(*i, 1, replacement, sizeof(replacement)));
		}
	}

	{ //0x005101BF
		short signature[] = {0xB9, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x8B, 0x0D, -1, -1, -1, -1, 0xA1};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 1, replacement, sizeof(replacement)));
	}

	{ //0x005101E0, 0x0050FE3E
		short signature[] = {0xB9, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x83, 0xC4, 0x14, 0xE8, -1, -1, -1, -1, 0xE8};
		CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
		for(auto i = cf.begin(); i != cf.end(); i++) {
			patches->add(new PatchHook(*i, 1, replacement, sizeof(replacement)));
		}
	}

	{ //0x00510017, 0x0050FF9C (write #2), 0x0050FFDE, 0x00510049 (specular)
		short signature[] = {0xB9, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0xA0, -1, -1, -1, -1, 0x83, 0xC4, 0x14, 0x3A, 0xC3};
		CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
		for(auto i = cf.begin(); i != cf.end(); i++) {
			patches->add(new PatchHook(*i, 1, replacement, sizeof(replacement)));
		}
	}
}

void increaseLimits() {
	uint16_t limit = 0x8000 - 1;
	short signature[] = {0x66, 0x81, 0x3D, -1, -1, -1, -1, 0x00, 0x40};
	CodeFinder cf(GetModuleHandle(0), signature, sizeof(signature) / 2);
	
	for(auto i = cf.begin(); i != cf.end(); i++) {
		patches->add(new PatchHook(*i, 7, &limit, sizeof(limit)));
	}
}

}};