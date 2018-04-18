#include "CustomChat.h"
#include "EnginePointers.h"
#include "EngineTypes.h"
#include "OpticMedal.h"
#include "Direct3D.h"
#include "EngineFunctions.h"
#include "console.h"
#include "Patcher.h"
#include "PatchGroup.h"
#include "DebugHelper.h"
#ifdef FONT_PRIV
#include "FontHandler.h"
#include "Preferences.h"
#endif
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9core.h>
#include <deque>
#include <algorithm>
#include <memory>
#include <sstream>
#include <concurrent_queue.h>

namespace Chat {

unsigned int maxTextLines;
bool bypass = false;
bool enabled = false;
bool redirect_feed = false;
LPD3DXFONT font;
UINT fontHeight;
LPD3DXSPRITE sprite;
std::uintptr_t ContinueChat;
std::deque<std::pair<Optic::OpticMedal, LPDIRECT3DTEXTURE9>> text;
Optic::OpticAnimation generateDefaultAnimation(int displayTime);
Optic::OpticAnimation generateDefaultSlideAnimation();
Optic::OpticAnimation defaultAnimation;
Optic::OpticAnimation defaultSlideAnimation = generateDefaultSlideAnimation();
std::unique_ptr<PatchGroup> patches = nullptr;
Concurrency::concurrent_queue<std::wstring> queuedText;
void chatHandler(const wchar_t* message, bool chat);

void install() {
	patches = std::unique_ptr<PatchGroup>(new PatchGroup());

	{
		short signature[] = {0xA1, -1, -1, -1, -1, 0x85, 0xC0, 0x74, 0x69};
		patches->add(new CaveHook(signature, sizeof(signature) / 2, 0, Chat::chatHandlerStub, CaveHook::NAKED, &Chat::ContinueChat));
	}

	{
		short signature[] = {0xA1, -1, -1, -1, -1, 0x85, 0xC0, 0x74, 0x66};
		std::uint8_t replacement[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xEB};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	{
		short signature[] = {0xA1, -1, -1, -1, -1, 0x85, 0xC0, 0x74, 0x6E};
		std::uint8_t replacement[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xEB};
		patches->add(new PatchHook(signature, sizeof(signature) / 2, 0, replacement, sizeof(replacement)));
	}

	if(!patches->install()) {
		patches.reset();
		throw CustomChatException("Failed to install hooks! Please ensure you're not running Halo Chat V2.");
	}
}

void uninstall() {
	patches.reset();
}

void createResources() {
	char* fontName = "";
	DWORD version = GetVersion();
    DWORD major = (DWORD) (LOBYTE(LOWORD(version)));

#ifdef FONT_PRIV
	if(major >= 6) {
		DWORD ret = FontHandler::loadFont(&fontName);

		if(ret != 0) {
			std::stringstream err;
			err << "Unable to load font! Code: " << ret;
			throw CustomChatException(err.str());
		}
	}
#endif
	
	fontHeight = Preferences::find("font_size", 30);

	if(!fontHeight) {
		fontHeight = static_cast<UINT>(34.0 * pResolution->height / 1440.0);
	}

	HRESULT res = D3DXCreateFont(D3DHook::pDevice, fontHeight, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
								 ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName, &font);

	if(res != S_OK) {
		std::stringstream err;
		err << "Unable to create D3D font. Code: " << res;
		throw CustomChatException(err.str());
	}

#ifdef FONT_PRIV
	if(major >= 6) {
		FontHandler::releaseFont();
	}
#endif

	res = D3DXCreateSprite(D3DHook::pDevice, &sprite);
	
	if(res != S_OK) {
		font->Release();
		std::stringstream err;
		err << "Unable to create D3D sprite. Code: " << res;
		throw CustomChatException(err.str());
	}
}

void resolutionChange() {
	if(enabled) {
		enable();
	}
}

void enable(bool chat_hide) {
	if(enabled) {
		disable();
	}

	try {
		createResources();
		install();

		std::uint16_t displayTime = Preferences::find<std::uint16_t>("display_time", 7);
		redirect_feed = Preferences::find<bool>("redirect_kill_feed", false);
		defaultAnimation = generateDefaultAnimation(displayTime * 1000);
		maxTextLines = Preferences::find<std::uint16_t>("line_limit", 6);

		enabled = true;
		bypass = chat_hide;
	} catch(CustomChatException& e) {
		hkDrawText("An error occurred while loading custom chat:", C_TEXT_RED);
		hkDrawText(e.what(), C_TEXT_RED);
	}
}

void clear() {
	for(auto i = text.begin(); i != text.end(); ++i) {
		i->second->Release();
	}

	text.clear();
}

void disable() {
	if(!enabled) {
		return;
	}

	bypass = false;
	enabled = false;
	uninstall();
	clear();
	font->Release();
	sprite->Release();
}

void lost() {
	if(enabled) {
		sprite->OnLostDevice();
		font->OnLostDevice();
	}
}

void reset() {
	if(enabled) {
		sprite->OnResetDevice();
		font->OnResetDevice();
		clear();
	}
}

void draw() {
	if(!enabled) {
		return;
	}

	//@todo - horrible, find a better way
	if(!queuedText.empty()) {
		std::wstring message;
		if(queuedText.try_pop(message)) {
			chatHandler(message.c_str(), false);
		}
	}

	if(sprite->Begin(D3DXSPRITE_ALPHABLEND) != S_OK) {
		MessageBox(NULL, "Direct3D failed to draw sprite!", 0, 0);
		return;
	}
	
	D3DXMATRIX mat, oldMat;
	D3DXMatrixIdentity(&mat);
	mat._12 = -0.040f;

	sprite->GetTransform(&oldMat);
	sprite->SetTransform(&mat);

	for(auto i = text.begin(); i != text.end();) {
		if(i->first.draw(sprite)) {
			++i;
		} else {
			i->second->Release();
			i = text.erase(i);
		}
	}

	sprite->SetTransform(&oldMat);

	if(sprite->End() != S_OK) {
		MessageBox(NULL, "Direct3D failed to draw sprite!", 0, 0);
	}
}

LPDIRECT3DTEXTURE9 renderTexture(const wchar_t* message, bool chat) {
	LPDIRECT3DTEXTURE9 texture = NULL;
	LPDIRECT3DSURFACE9 pRenderSurface = NULL, pBackBuffer = NULL;
	D3DXMATRIX projection, oldProjection;

	D3DHook::pDevice->CreateTexture(pResolution->width, fontHeight, 1, D3DUSAGE_RENDERTARGET,
									D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL);
	texture->GetSurfaceLevel(0, &pRenderSurface);
	D3DHook::pDevice->GetTransform(D3DTS_PROJECTION, &oldProjection);
	D3DHook::pDevice->GetRenderTarget(0, &pBackBuffer);
	D3DHook::pDevice->SetRenderTarget(0, pRenderSurface);
	D3DHook::pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
	
	D3DHook::pDevice->BeginScene();
	RECT font_rect;
	SetRect(&font_rect, 0, 0, 0, 0);

	D3DCOLOR colour = chat? D3DCOLOR_RGBA(178, 225, 251, 255) : D3DCOLOR_RGBA(139, 190, 247, 255);
	font->DrawTextW(NULL, message, -1, &font_rect, DT_LEFT | DT_NOCLIP, colour);
	D3DHook::pDevice->EndScene();

	D3DHook::pDevice->SetRenderTarget(0, pBackBuffer);
	D3DHook::pDevice->SetTransform(D3DTS_PROJECTION, &oldProjection);

	pBackBuffer->Release();
	pRenderSurface->Release();

	return texture;
}

void queueText(LPDIRECT3DTEXTURE9 texture) {
	Optic::OpticSprite sprite(texture, defaultAnimation);
	sprite.setTransformationCentre(0.0f, 0.0f);
	Optic::OpticMedal line(sprite);
	line.setSlideAnimation(defaultSlideAnimation);

	std::for_each(text.begin(), text.end(), 
		[&line](std::pair<Optic::OpticMedal, LPDIRECT3DTEXTURE9>& active) {
			if(line.group().group == active.first.group().group && !active.first.ending()) {
				active.first.pushNotify(line.dimensions());
			}
	});

	//@todo - critical section
	text.emplace_back(std::make_pair(line, texture));

	//@todo - RAII wrapper & erase
	while(text.size() > maxTextLines) {
		auto& medal = text[0];
		medal.second->Release();
		text.pop_front();		
	}
}

void chatHandler(const wchar_t* message, bool chat) {
	if(!chat && redirect_feed) { // todo, fix this hacky stuff
		HUDMessage(message);
		return;
	}

	if(!enabled || bypass) {
		return;
	}

	if(D3DHook::pDevice->TestCooperativeLevel() != D3D_OK) {
		return;
	}

	LPDIRECT3DTEXTURE9 texture = renderTexture(message, chat);
	queueText(texture);
}

void medalText(const std::string& message) {
	if(!enabled) {
		return;
	}

	std::wstring wmessage;
	wmessage.assign(message.begin(), message.end());
	queuedText.push(wmessage);
}

void __declspec(naked) chatHandlerStub() {
	__asm {
		pushad
		pushfd
		xor ecx, ecx
		test ebx, ebx
		jne not_chat
		mov cl, 1
		not_chat:
		lea ebx, [esp + 0x30]
		mov ebx, [ebx]
		push ecx
		push ebx
		call chatHandler
		add esp, 8
		popfd
		popad
		xor eax, eax
		jmp ContinueChat
	}
}

using namespace Optic;

Optic::OpticAnimation generateDefaultAnimation(int displayTime) {
	OpticAnimation animation;
	animation.addKeyframe(Keyframe(0, 0.015f), KEYFRAME_POSITION_X);
	animation.addKeyframe(Keyframe(0, 0.55f), KEYFRAME_POSITION_Y);
	animation.addKeyframe(Keyframe(0, 0.0f), KEYFRAME_OPACITY);
	animation.addKeyframe(Keyframe(200, 1.0f), KEYFRAME_OPACITY);
	animation.addKeyframe(Keyframe(displayTime, 1.0f), KEYFRAME_OPACITY);
	animation.addKeyframe(Keyframe(displayTime + 250, 0.0f), KEYFRAME_OPACITY);
	animation.addKeyframe(Keyframe(0, 0.0f), KEYFRAME_SCALE);
	animation.addKeyframe(Keyframe(265, 2.0f), KEYFRAME_SCALE);
	animation.addKeyframe(Keyframe(460, 1.0f), KEYFRAME_SCALE, EASE_IN);
	return animation;
}

Optic::OpticAnimation generateDefaultSlideAnimation() {
	OpticAnimation animation;
	animation.addKeyframe(Keyframe(0, 0.0f), KEYFRAME_POSITION_Y);
	animation.addKeyframe(Keyframe(400, 1.0f), KEYFRAME_POSITION_Y, EASE_IN);
	return animation;
}

};