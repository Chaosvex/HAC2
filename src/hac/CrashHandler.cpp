#include "CrashHandler.h"
#include "resource.h"
#include "misc.h"
#include "DebugHelper.h"
#include <iomanip>
#include <sstream>
#include <bitset>
#include <tchar.h>
#include <Psapi.h>
#include <vector>
#include <fstream>

struct MODULE_REPORT {
	char* name;
	DWORD base, entry, size;
} MOD_REP;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::string CodeString(DWORD code);
std::vector<std::string> walkStack(DWORD ebp);
void temp();
std::string generateReport(_EXCEPTION_POINTERS* info);
std::string saveReport(std::string report);
HMODULE locateFaultingModule(DWORD* faultAddr);

LONG WINAPI crashHandler(_EXCEPTION_POINTERS* info) {
	std::string filename = saveReport(generateReport(info));

	HWND hDlg = CreateDialogParam(GetModuleHandle("hac.dll"), MAKEINTRESOURCE(IDD_CRASH), 0, DialogProc, 0);

	if(hDlg == NULL) {
		return EXCEPTION_EXECUTE_HANDLER;
	}

	//Generate summary text
	char name[256];
	char *baseName = NULL;
	std::string final = "<unknown>";

	HMODULE module = locateFaultingModule((DWORD*)&info->ExceptionRecord->ExceptionAddress); 

	if(module != NULL) {
		if(GetModuleFileName(module, name, sizeof(name)) != 0) {
			baseName = strrchr(name, '\\') + 1;
			
			if(baseName) {
				final = baseName;
			}
		}
	}

	std::stringstream summary;
	summary << CodeString(info->ExceptionRecord->ExceptionCode) << " in " << final
		<< " occured at 0x" << info->ExceptionRecord->ExceptionAddress;

	std::stringstream display;
	display << "Halo has encountered a fatal error and needs to close." << "\r\n\r\n";
	display << "Error report file:" << "\r\n";
	display << filename;

	SetDlgItemText(hDlg, IDC_REPORT, display.str().c_str());
	SetDlgItemText(hDlg, IDC_ERROR_SUMMARY, summary.str().c_str());

	ShowWindow(hDlg, SW_SHOW);

	BOOL ret;
	MSG msg;
	while((ret = GetMessage(&msg, 0, 0, 0)) != 0 && ret != -1) {
		if(!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_OK:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return TRUE;
				case ID_REJOIN:
					temp();
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return TRUE;
				case ID_SEND:
					temp();
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return TRUE;
			}
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

std::string CodeString(DWORD code) {
	//Retrieval for this code doesn't work (?!)
	if(code == EXCEPTION_ACCESS_VIOLATION) {
		return "Access violation";	
	}

	char buffer[256];
	HMODULE mod = LoadLibrary("ntdll.dll");
	FormatMessage(FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
		mod, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL);
	return std::string(buffer);
}

void temp() {
	MessageBoxA(NULL, "Not implemented.", "Not implemented.", 0);
}

std::ostream& field(std::ostream& o) {
    return o << std::setw(10) << std::right;
}

std::ostream& regfield(std::ostream& o) {
    return o << std::setfill(' ') << std::setw(3) << std::right;
}

std::ostream& hexfield(std::ostream& o) {
    return o << std::setfill('0') << std::setw(8) << std::right;
}

std::string generateReport(struct _EXCEPTION_POINTERS* info) {
	char name[256];
	char *baseName = NULL;
	MODULEINFO modInfo;
	std::stringstream report;

	report << "EXCEPTION INFORMATION" << std::endl;
	report << field << "Code: " << "0x" << std::hex << info->ExceptionRecord->ExceptionCode << std::endl;
	report << field << "Address: " << "0x" << std::hex << info->ExceptionRecord->ExceptionAddress << std::endl;
	report << field << "Flags: " << info->ExceptionRecord->ExceptionFlags << std::endl << std::endl;
	
	report << "MODULE INFORMATION" << std::endl;
	HMODULE module = locateFaultingModule((DWORD*)&info->ExceptionRecord->ExceptionAddress); 

	if(module != NULL) {
		if(GetModuleFileName(module, name, sizeof(name)) != 0) {
			baseName = strrchr(name, '\\') + 1;
		}
	}

	if(baseName != NULL) {
		report << field << "Name: " << baseName << std::endl;
	} else {
		report << field << "Name: " << "Unknown" << std::endl;
	}

	if(module != NULL && GetModuleInformation(GetCurrentProcess(), module, &modInfo, sizeof(modInfo)) != 0) {
		report << field << "Base: " << "0x" << modInfo.lpBaseOfDll << std::endl;
		report << field << "Entry: " << "0x" << modInfo.EntryPoint << std::endl;
		report << field << "Size: " << "0x" << modInfo.SizeOfImage << std::endl;
		report << field << "Offset: " << "0x" << (DWORD)info->ExceptionRecord->ExceptionAddress
			- (DWORD)modInfo.lpBaseOfDll << std::endl << std::endl;
	} else {
		report << "Failed to gather module information!" << std::endl << std::endl;
	}
	
	report << "CONTEXT INFORMATION" << std::endl;
	report << regfield << "EIP " << "0x" << hexfield << info->ContextRecord->Eip
		<< regfield << "  EAX " << "0x" << hexfield << info->ContextRecord->Eax << std::endl;
	report << regfield << "ESP " << "0x" << hexfield << info->ContextRecord->Esp
		<< regfield << "  EBP " << "0x" << hexfield << info->ContextRecord->Ebp << std::endl;
	report << regfield << "EBX " << "0x" << hexfield << info->ContextRecord->Ebx
		<< regfield << "  ECX " << "0x" << hexfield << info->ContextRecord->Ecx << std::endl;
	report << regfield << "EDI " << "0x" << hexfield << info->ContextRecord->Edi
		<< regfield << "  EDX " << "0x" << hexfield << info->ContextRecord->Edx << std::endl;
	report << regfield << "ESI " << "0x" << hexfield << info->ContextRecord->Esi << std::endl << std::endl;

	std::bitset<32> f((int)info->ContextRecord->EFlags);
	report << "EFLAGS:" << std::endl;
	report << "CF " << f[0] << " PF " << f[2] << " AF " << f[4] << " ZF " << f[6] << std::endl;
	report << "SF " << f[7] << " TF " << f[8] << " OF " << f[11] << std::endl << std::endl;

	report << regfield << "DR0 " << "0x" << hexfield << info->ContextRecord->Dr0
		<< regfield << "  DR1 " << "0x" << hexfield << info->ContextRecord->Dr1 << std::endl;
	report << regfield << "DR2 " << "0x" << hexfield << info->ContextRecord->Dr2
		<< regfield << "  DR3 " << "0x" <<hexfield << info->ContextRecord->Dr3 << std::endl;
	report << regfield << "DR6 " << "0x" << hexfield << info->ContextRecord->Dr6 
		<< regfield << "  DR7 " << "0x" << hexfield << info->ContextRecord->Dr7 << std::endl << std::endl;

	report << "STACK INFORMATION" << std::endl;
	
	std::vector<std::string> trace = walkStack(info->ContextRecord->Ebp);
	
	for(int i = trace.size(), j = 0; j < i; j++) {
		report << trace[j] << std::endl;
	}

	if(trace.empty()) {
		report << "Unable to produce stack trace." << std::endl;
	}

	return report.str();
}

std::string saveReport(std::string report) {
	SYSTEMTIME time;
	GetSystemTime(&time);
	std::stringstream file, path;
	file << time.wDay << "_" << time.wMonth << "_" << time.wYear << "_" <<
		time.wHour << "_" << time.wMinute << "_" << time.wSecond << ".txt";
	path << getWorkingDir() << file.str();
	std::ofstream out;
	out.open(path.str().c_str());
	out << report;
	out.close();
	return file.str();
}

HMODULE locateFaultingModule(DWORD* faultAddr) {
	HMODULE modules[256];
	HANDLE process = GetCurrentProcess();
	DWORD required, entries;
	EnumProcessModules(process, modules, sizeof(modules), &required);
	entries = required / sizeof(HMODULE*);

	for(unsigned int i = 0; i < entries; i++) {
		MODULEINFO info;
		GetModuleInformation(process, modules[i], &info, sizeof(MODULEINFO));
		__try {
			if(*faultAddr >= (DWORD)info.lpBaseOfDll && *faultAddr <= ((DWORD)info.lpBaseOfDll + info.SizeOfImage)) {
				return modules[i];
			}
		} __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION?
				EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
			return NULL;
		}
	}

	return NULL;
}

/* 
 * A simple function to walk the EBP chain
 * and return basic information on the stack
 */
std::vector<std::string> walkStack(DWORD ebp) {
    char name[256];
	std::stringstream base;
	std::vector<std::string> trace;

    while(ebp != 0) {
		HMODULE mod = locateFaultingModule(reinterpret_cast<DWORD*>(ebp + 4));

		//hacky fix
		if(!mod) {
			return trace;
		}

		if(GetModuleFileName(mod, name, sizeof(name)) != 0) {
			base << std::setfill(' ') << std::setw(12) << std::left << strrchr(name, '\\') + 1;
		} else {
			base << "Unknown module";
		}
		base << " ret: " << std::hex << "0x" << hexfield << (*reinterpret_cast<DWORD*>(ebp + 4));
		trace.emplace_back(base.str());
		base.str(std::string());
		base.clear();
		ebp = *reinterpret_cast<DWORD*>(ebp);
    }
	
    return trace;
}