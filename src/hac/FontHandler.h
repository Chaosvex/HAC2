#pragma once

#include <Windows.h>

namespace FontHandler {

DWORD loadFont(char** name);
void releaseFont();

};