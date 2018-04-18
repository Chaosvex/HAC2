#pragma once

#include <Windows.h>
#include <string>

extern LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT *pNumArgs);
BOOL IsProcessElevated();
std::string getWorkingDir();