#include "DebugHelper.h"

DWORD WINAPI FreeAndLoad(void* buffer);

void DebugHelper::Translate(LPTSTR lpszFunction) {
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

	if(lpDisplayBuf == NULL) {
		OutputDebugString("Memory allocation failed!");
		return;
	}

	StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
	
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);	
}

void DebugHelper::DisplayAddress(DWORD address, short radix) {
	char buff[10];
	_itoa_s(address, buff, 10, radix);
	MessageBox(NULL, buff, buff, 0);
	OutputDebugString(buff);
}