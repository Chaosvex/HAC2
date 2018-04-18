#pragma once

#include <Windows.h>
#include <string>
#include <strsafe.h>

using std::string;

class DebugHelper {

public:
	static void Translate(LPTSTR lpszFunction);
	static void DisplayAddress(DWORD address, short radix);
	static DWORD WINAPI UnloadHAC(void*);
	static void ReloadHAC();
};