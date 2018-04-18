#pragma once

#include <vector>
#include <Windows.h>
#include "IHook.h"

using namespace std;

class CaveHook : public IHook {
public:
	CaveHook(const char* signature, size_t instrLen,  DWORD Function, DWORD *BackAddress)
	: size(instrLen), BackAddress(BackAddress), Function(Function), signature(signature), trampoline(false) {}
	CaveHook(const char* signature, size_t instrLen,  DWORD Function, DWORD *BackAddress,
				 bool trampoline, DWORD offset) : size(instrLen), BackAddress(BackAddress), Function(Function),
				 signature(signature), trampoline(trampoline), jmpOffset(offset){}
	~CaveHook();
	bool Install();

private:
	void Uninstall();
	size_t size;
	char OrginalCode[16];
	DWORD* BackAddress;
	DWORD Function;
	long PatchAddress;
	const char* signature;
	bool trampoline;
	DWORD jmpOffset;
};

class PatchHook : public IHook {
public:
	PatchHook(const char* signature, int offset, void* replacement, size_t instrLen, int instances = 1)
	: size(instrLen), replacement(replacement), offset(offset), signature(signature) {}
	~PatchHook();
	bool Install();

private:
	void Uninstall();
	size_t size;
	char OrginalCode[16];
	int offset;
	void* replacement;
	long PatchAddress;
	const char* signature;
};