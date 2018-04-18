#include "Downloader.h"
#include "happyhttp\happyhttp.h"
#include "InitHooks.h"
#include "Shared.h"
#include "DebugHelper.h"
#include "resource.h"
#include <string>
#include <sstream>
#include <Windows.h>
#include <shlobj.h>

using namespace std;
using namespace happyhttp;

#define PATH_BUFFER_SIZE 512

bool LoadLocal();
bool DownloadDLL(bool useEtag, char* etag);
void SetTemporaryPath();
void LaunchBlock();
DWORD _stdcall DownloadThread(void* o);
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ErrorDisplay(char* error, unsigned int code);
HANDLE downloadThread;
DWORD ContinueLaunch;
char oldCurrentDirectory[PATH_BUFFER_SIZE];
void RestoreOldPath();
DWORD* dwModuleBase = 0, Register_ret = 0;
FARPROC test = 0;

enum DownloadStatus {
	DOWNLOAD_SUCCESS,
	DOWNLOAD_THREAD_FAILURE,
	DOWNLOAD_FAILURE
};

void OnRegister(LPDWORD lpModuleBase) {
    *lpModuleBase = (DWORD)GetModuleHandle("controls.dll");
}

extern "C" __declspec(dllexport, naked) bool Register() {
    __asm {
		pop Register_ret
		mov dwModuleBase, ebx
		add dwModuleBase, 20h
	}

    OnRegister(dwModuleBase);

    __asm {
		push Register_ret
		mov al, 0
		ret
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ulReason, LPVOID lpReserved) {
	/* Halo loads all DLLs in /controls each time the window is lost, causing the reference count for
	   resident DLLs like HAC to increment; this causes issues when unloading. Solution is to avoid
	   calling LoadLibrary if the DLL is already loaded */
	if(ulReason == DLL_PROCESS_ATTACH && GetModuleHandle("hac.dll") == NULL) {
		if(!LoadLocal()) {
			try {
				InitialiseHooks();
				downloadThread = CreateThread(NULL, NULL, DownloadThread, NULL, NULL, NULL);
				if(downloadThread == NULL) {
					ErrorDisplay("Unable to load HAC!", GetLastError());	
				}
			} catch(HookException& e) {
				OutputDebugString(e.what());
				MessageBoxW(NULL, L"Halo Anticheat detected a compatibility issue and will be unavailable!",
					L"Error!", MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}

	return TRUE;
}

void _declspec(naked) LaunchBlockCave() {
	__asm {
		pushfd
		pushad
		call LaunchBlock
		call UninstallHooks
		popad
		popfd

		//Original code
		mov eax, dword ptr ds:[ebx+20]
		add esp, 4
		jmp ContinueLaunch
	}
}
ULONG_PTR EnableVisualStyles();

void LaunchBlock() {
	DWORD began = GetTickCount();
	HRESULT result = WaitForSingleObject(downloadThread, 5000); //Wait for the init thread to complete
	DWORD elapsed = GetTickCount() - began;
	DWORD exitCode = 1;

	//if(result == WAIT_TIMEOUT) {
	//	EnableVisualStyles(); //because nothing else worked
	//	HWND window = CreateDialog(GetModuleHandle("loader.dll"), MAKEINTRESOURCE(IDD_FORMVIEW), NULL, DialogProc);
	//	MSG msg;
	//	ShowWindow(window, SW_SHOW);

	//	while(elapsed < 180000) {
	//		elapsed = GetTickCount() - began;
	//		DWORD progress = (DWORD)(elapsed / 180000.0f * 100);

	//		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
	//			if(!IsDialogMessage(window, &msg)) {
	//				TranslateMessage(&msg);
	//				DispatchMessage(&msg);
	//			}
	//		}

	//		SendMessage(GetDlgItem(window, IDC_PROGRESS1), PBM_SETPOS, 100 - progress, 0);
	//		stringstream status;
	//		status << "Update timing out in " << 180 - (elapsed / 1000) << " seconds...";
	//		SetDlgItemText(window, IDC_STATUS_LABEL, status.str().c_str());
	//		
	//		if(GetExitCodeThread(downloadThread, &exitCode) == TRUE && exitCode == STILL_ACTIVE) {
	//			Sleep(5);
	//		} else {
	//			break;
	//		}
	//	}

	//	DestroyWindow(window);
	//	TerminateThread(downloadThread, 0);
	//	CloseHandle(downloadThread);
	//}

	//Load DLL
	char buffer[PATH_BUFFER_SIZE];
	GetTempPath(PATH_BUFFER_SIZE, buffer);
	strcat_s(buffer, PATH_BUFFER_SIZE, "hac.dll");

	if(LoadLibrary(buffer) == NULL) {
		ErrorDisplay("Unable to load HAC!", GetLastError());
	}
}

void ErrorDisplay(char* error, unsigned int code) {
	stringstream text;
	text << error << " Error code: " << code;
	//MessageBoxA(NULL, text.str().c_str(), "Error!", 0);
}

bool LocateEtag(char* etag) {
	FILE* handle;

	//Ensure the DLL exists first
	if(fopen_s(&handle, "hac.dll", "r") != 0) {
		return false;
	}

	fclose(handle);

	//Check to see if the Etag file exists
	if(fopen_s(&handle, "hac.tag", "rb") != 0) {
		return false;
	}

	//Load Etag
	fseek(handle, 0, SEEK_END);
	size_t size = ftell(handle);
	rewind(handle);
	fread_s(etag, 64, size, 1, handle);
	fclose(handle);

	return true;
}

DWORD _stdcall DownloadThread(void*) {
	DownloadStatus ret = DOWNLOAD_SUCCESS;
	char etag[64] = {0};
	SetTemporaryPath();
	bool useEtag = LocateEtag(etag);
	RestoreOldPath();

	//Hacky McHack until I rewrite this rubbish
	try {
		if(!DownloadDLL(useEtag, etag)) {
			ret = DOWNLOAD_FAILURE;
		}
	} catch(...) {
		ret = DOWNLOAD_FAILURE;
	}

	return ret;
}

bool LoadLocal() {
	TCHAR buffer[256];
	GetFullPathName("hac.dll", 256, buffer, NULL);
	return (LoadLibraryA(buffer) != NULL);
}

void RestoreOldPath() {
	SetCurrentDirectory(oldCurrentDirectory);
}

void SetTemporaryPath() {
	char buffer[PATH_BUFFER_SIZE];
	GetCurrentDirectory(PATH_BUFFER_SIZE, oldCurrentDirectory);
	GetTempPath(PATH_BUFFER_SIZE, buffer);
	SetCurrentDirectory(buffer);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
		case WM_COMMAND:
			break;
		case WM_CLOSE:
			DestroyWindow(hDlg);
			return TRUE;
		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
	}
	return FALSE;
}

bool DownloadDLL(bool useEtag, char* etag) {
	char source[PATH_BUFFER_SIZE], dest[PATH_BUFFER_SIZE];
	GetTempPath(PATH_BUFFER_SIZE, source);
	GetTempPath(PATH_BUFFER_SIZE, dest);
	strcat_s(source, PATH_BUFFER_SIZE, "hac.tmp");
	strcat_s(dest, PATH_BUFFER_SIZE, "hac.dll");

	Downloader download("hac.dll", "hac.tmp");
	bool success = true;
	
	if(useEtag) {
		download.setEtag(etag);
	}

	try {
		download.Go();

		while(download.Pump()) {
			//erp
		}
	} catch(Wobbly& e) {
		//MessageBox(NULL, e.what(), "Update error!", 0); // nice fix!
		success = false;
	}

	if(success) {
		MoveFileEx(source, dest, MOVEFILE_REPLACE_EXISTING);
	}

	return success;
}

ULONG_PTR EnableVisualStyles(VOID)
{
    TCHAR dir[MAX_PATH];
    ULONG_PTR ulpActivationCookie = FALSE;
    ACTCTX actCtx =
    {
        sizeof(actCtx),
        ACTCTX_FLAG_RESOURCE_NAME_VALID
            | ACTCTX_FLAG_SET_PROCESS_DEFAULT
            | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID,
        TEXT("shell32.dll"), 0, 0, dir, (LPCTSTR)124
    };
    UINT cch = GetSystemDirectory(dir, sizeof(dir) / sizeof(*dir));
    if (cch >= sizeof(dir) / sizeof(*dir)) { return FALSE; /*shouldn't happen*/ }
    dir[cch] = TEXT('\0');
    ActivateActCtx(CreateActCtx(&actCtx), &ulpActivationCookie);
    return ulpActivationCookie;
}
