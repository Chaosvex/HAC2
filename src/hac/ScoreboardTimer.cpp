#include "ScoreboardTimer.h"
#include "ServerInformation.h"
#include "EngineTypes.h"
#include "Patcher.h"
#include "PatchGroup.h"
#include "Shared.h"
#include <memory>
#include <cstdint>
#include "console.h"

namespace scoreboard {

std::uintptr_t BackToScoreboardTop;
std::uintptr_t BackToScoreboardHeader;
std::uintptr_t BackToScoreboard;
std::uintptr_t BackToScoreboardStar;

std::unique_ptr<PatchGroup> patches = nullptr;

const wchar_t* __stdcall FormatNameTopString(wchar_t* text) {
	//if (serverInfo.type == ServerDetails::Type::SAPP && serverInfo.mod_version >= 0x09070100) {
		std::size_t len = wcslen(text);

		if (len && text[len - 1] == ' ') {
			text[--len] = 0;
		}

		if (timeleft > 0) {
			int tmp = timeleft / 30;
			swprintf(text + len, L", Time Remaining: %d:%02d", tmp / 60, tmp % 60);
		} else {
			wcscpy(text + len, L", Time Remaining: Infinite");
		}
	//}

	return text;
}

void __declspec(naked) ScoreBoardTopCC() {
	__asm {
		mov eax, dword ptr ss : [esp + 0x350]
		push eax
		call FormatNameTopString
		mov ecx, eax
		jmp BackToScoreboardTop
	}
}

std::wstring wstrnh;
const wchar_t* __stdcall FormatNameHeaderString(wchar_t* name) {
	//wstrnh = FormatWArgs(L"%ls [Sapp: %ls] [AC: %ls]", name, SappServer ? L"Yes" : L"No", ACServer ? L"ON" : L"OFF");
	return wstrnh.c_str();
}

void __declspec(naked) ScoreBoardHeaderCC() {
	__asm {
		mov eax, dword ptr ss : [esp + 0x80]
		push ecx
		push ebx
		push edx
		push esi
		push edi
		push eax		
		call FormatNameHeaderString
		pop edi
		pop esi
		pop edx
		pop ebx
		pop ecx
		jmp BackToScoreboardHeader
	}
}

std::wstring wstrn;
const wchar_t* __stdcall FormatNameString(wchar_t* name, size_t index) {
	index /= 0x200;
	//wstrn = FormatWArgs(L"%ls - %hs", name, ACPlayers[index]);
	return wstrn.c_str();
}

void __declspec(naked) ScoreBoardCC() {
	__asm {
		push eax
		push ebx
		push edx
		push esi
		push edi
		lea ecx, [ebp + ecx + 4]
		push ebp
		push ecx
		call FormatNameString
		mov ecx, eax
		pop edi
		pop esi
		pop edx
		pop ebx
		pop eax
		push ecx
		jmp BackToScoreboard
	}
}

void __declspec(naked) ScoreBoardStarCC() {
	__asm {
		push eax
		push ebx
		push edx
		push esi
		push edi
		lea ecx, [ebp + esi + 4]
		push ebp
		push ecx
		call FormatNameString
		mov ecx, eax
		pop edi
		pop esi
		pop edx
		pop ebx
		pop eax
		push ecx
		jmp BackToScoreboardStar
	}
}

void on_tick() {
	if(timeleft > 1) {
		--timeleft;
	}
}

void __declspec(noinline) install() {
	patches = std::unique_ptr<PatchGroup>(new PatchGroup());
	
	{ // ScoreBoardTopSig
		short signature[] = {0x8B, 0x8C, 0x24, 0x50, 0x03, 0x00, 0x00, 0x5F, 0x5E, 0x5D, 0x66, 0xC7, 0x81, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 8B8C24500300005F5E5D66C7819E0000000000
		patches->add(new CaveHook(signature, sizeof(signature) / 2, 0, ScoreBoardTopCC, CaveHook::NAKED, &BackToScoreboardTop));
	}

	{ // ScoreBoardHeaderSig
		short signature[] = { 0x8B, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00, 0x8B, 0x4C, 0x24, 0x7C }; // 8B8424800000008B4C247C
		//patches->add(new CaveHook(signature, sizeof(signature) / 2, 0, ScoreBoardTopCC, CaveHook::JMP_TP, &BackToScoreboardTop));
	}

	{ // ScoreBoardSig
		short signature[] = { 0x56, 0x50, 0x53, 0x8D, 0x4C, 0x29, 0x04, 0x51, 0x52 }; // 5650538D4C29045152
		//patches->add(new CaveHook(signature, sizeof(signature) / 2, 3, ScoreBoardTopCC, CaveHook::JMP_TP, &BackToScoreboardTop));
	}

	{ // ScoreBoardStarSig
		short signature[] = { 0x51, 0x50, 0x53, 0x8D, 0x4C, 0x2E, 0x04, 0x51, 0x52 }; // 5150538D4C2E045152
		//patches->add(new CaveHook(signature, sizeof(signature) / 2, 3, ScoreBoardTopCC, CaveHook::JMP_TP, &BackToScoreboardTop));
	}

	if(!patches->install()) {
		restore();
		throw HookException("Unable to install scoreboard hooks!");
	}
}

void restore() {
	patches.reset();
}

} // scoreboard