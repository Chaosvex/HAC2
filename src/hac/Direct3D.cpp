#include "Direct3D.h"
#include "Optic.h"
#include "Sightjacker.h"
#include "CustomChat.h"
#include "Preferences.h"
#include "SettingsUIEnums.h"
#include <d3dx9.h>
#include <string>

namespace D3DHook {

LPDIRECT3DDEVICE9 pDevice;
tEndScene originalEndScene = NULL;
tReset originalReset = NULL;

//temporary
void delayedInit() {
	std::string pack = Preferences::find<std::string>("optic_pack", "");
	std::uint16_t chatRenderer = Preferences::find("custom_chat", 0);

	if(!pack.empty()) {
		Optic::load(pack);
	}

	switch(chatRenderer) {
		case SettingsUI::ChatRenderer::HAC2:
			Chat::enable();
			break;
		case SettingsUI::ChatRenderer::HIDE:
			Chat::enable(true);
			break;
	}
}

HRESULT __stdcall endScene(LPDIRECT3DDEVICE9 device) {
	pDevice = device;
	Optic::render();
	SightJacker::draw();
	Chat::draw();

	//temporary
	static bool firstEnd = true;

	if(firstEnd) {
		firstEnd = false;
		delayedInit();
	}

	return originalEndScene? originalEndScene(device) : device->EndScene();
}

HRESULT __stdcall reset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	pDevice = device;
	Optic::lost();
	SightJacker::lost();
	Chat::lost();
	HRESULT res;

	if(!originalReset) {
		res = device->Reset(pPresentationParameters);
	} else {
		res = originalReset(device, pPresentationParameters);
	}

	Optic::reset();
	SightJacker::reset();
	Chat::reset();

	if(!originalReset) {
		__asm {
			mov eax, res
			test eax, eax
		}
	}

	return res;
}

};