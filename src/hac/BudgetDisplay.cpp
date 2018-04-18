#include "BudgetDisplay.h"
#include "EngineFunctions.h"
#include "console.h"
#include "Language.h"
#include <Windows.h>
#include <cstdio>
#include <process.h>

short *polyCount;

namespace {
	HANDLE dThread = NULL;
	bool stop = false;
	void __cdecl display(void*);
}

void displayBudgetStart() {
	if(dThread == NULL) {
		dThread = (HANDLE)_beginthread(display, 0, NULL);
		if(dThread == NULL) {
			hkDrawText(Language::GetString(BUDGET_START_ERROR), C_TEXT_YELLOW);
		}
	}
}

void displayBudgetStop() {
	if(dThread == NULL) {
		return;
	}

	stop = true;
	HRESULT res = WaitForSingleObject(dThread, 2000);

	if(SUCCEEDED(res)) {
		dThread = NULL;
		stop = false;
	} else {
		hkDrawText(Language::GetString(BUDGET_STOP_ERROR), C_TEXT_YELLOW);
	}
}

namespace {

void __cdecl display(void*) {
	char buffer[128];
	double percentage1 = 0, percentage2 = 0;

	while(!stop) {
		/* Fixes race condition where 0 is written to the value at the start of each
		   new frame render, causing the counter to look glitchy at high framerates.
		   Doesn't fix the possibility that it may still be counting upwards when the
		   thread updates the display.
		   No real need to install a hook for sync for a minor issue. */
		if(*polyCount == 0) {
			Sleep(10);
			continue;
		}

		percentage1 = (*polyCount / 16384.0) * 100;

		if(percentage1 >= 100) {
			HUDMessage(Language::GetString(BUDGET_EXCEED));
		} else {
			HUDMessage(L"");
		}

		percentage2 = (*polyCount / 32768.0) * 100;

		if(percentage2 > 100) {
			HUDMessage(Language::GetString(BUDGET_HAC_EXCEED));
		} else {
			HUDMessage(L"");
		}
		
		sprintf_s(buffer, 128, Language::GetString(BUDGET_FORMATTER).c_str(), *polyCount, percentage1);
		HUDMessage(buffer);
		sprintf_s(buffer, 128, Language::GetString(BUDGET_HAC_FORMATTER).c_str(), *polyCount, percentage2);
		HUDMessage(buffer);
		Sleep(33);
	}
}

}