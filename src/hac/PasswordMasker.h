#pragma once

#include <cstdint>

extern std::uintptr_t ContinueTerminalInput;
char PasswordMasker(char* buffer, char currChar, int position);
void PasswordMaskerStub();