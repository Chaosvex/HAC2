#include "Language.h"
#include <Windows.h>

namespace Language {

std::string GetString(unsigned int identifier) {
	HMODULE handle = GetModuleHandle("hac.dll");
	char buff[256];

	if(LoadStringA(handle, identifier, buff, 256) == 0) {
		return std::string("TRANSLATION MISSING!");
	}

	return std::string(buff);
}

std::wstring GetWideString(unsigned int identifier) {
	HMODULE handle = GetModuleHandle("hac.dll");
	wchar_t buff[256];

	if(LoadStringW(handle, identifier, buff, 256) == 0) {
		return std::wstring(L"TRANSLATION MISSING!");
	}

	return std::wstring(buff);
}

void SetLanguage(unsigned int localeID) {
	
}

}