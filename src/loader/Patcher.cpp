#include "Patcher.h"
#include "codefinder.h"

CaveHook::~CaveHook() {
	Uninstall();
}

bool CaveHook::Install() {
	try {
		PatchAddress = FindCode((char*)GetModuleHandle("keystone.dll"), string(signature));
	} catch(exception) {
		return false;
	}

	if(PatchAddress == NULL) {
		return false;
	}

	char *cc_code = (char*)PatchAddress;
	DWORD OldProtect;

	memcpy(OrginalCode, cc_code, size);

	*BackAddress = PatchAddress + size;

	// insert codecave jump
	if(VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect) == 0) {
		return false;
	}

	cc_code[0] = '\xE9'; // opcode of jmp
	DWORD CallOffset = (DWORD)Function-(PatchAddress+5);
	*(DWORD*)(&cc_code[1]) = CallOffset;

	//Calculate how many bytes of the original instruction are remaining and
	//then overwrite them with nops
	DWORD bytesLeft = size - 5; //a jmp hook is five bytes
	memset(&cc_code[5], 0x90, bytesLeft);

	//Restore original page protection
	VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);

	if(trampoline) {
		*(DWORD*)(&cc_code[1]) = PatchAddress + size;
		if(VirtualProtect((LPVOID)(Function + jmpOffset), 5, PAGE_EXECUTE_READWRITE, &OldProtect) == 0) {
			return false;
		}
		memcpy((void*)(Function + jmpOffset), cc_code, 5);
	}

	return true;
}

void CaveHook::Uninstall() {
	DWORD OldProtect;
	char *code = (char*)PatchAddress;
	VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);	
	memcpy(code, OrginalCode, size);
	VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
}

PatchHook::~PatchHook() {
	Uninstall();
}

bool PatchHook::Install() {
	PatchAddress = FindCode((char*)GetModuleHandle(0), string(signature));

	if(PatchAddress == NULL) {
		return false;
	}

	PatchAddress += offset;
	
	//Back current code up
	memcpy(OrginalCode, (char*)PatchAddress, size);

	DWORD oldProtect = 0;
	VirtualProtect((void*)PatchAddress, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((void*)PatchAddress, replacement, size);
	VirtualProtect((void*)PatchAddress, size, oldProtect, &oldProtect);
	return true;
}

void PatchHook::Uninstall() {
	DWORD OldProtect;
	char *code = (char*)PatchAddress;
	VirtualProtect((LPVOID)PatchAddress, size, PAGE_EXECUTE_READWRITE, &OldProtect);	
	memcpy(code, OrginalCode, size);
	VirtualProtect((LPVOID)PatchAddress, size, OldProtect, &OldProtect);
}