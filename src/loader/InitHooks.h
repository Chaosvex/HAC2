#pragma once

#include "PatchGroup.h"
#include <exception>
#include <string>
#include <vector>

using namespace std;

void InitialiseHooks();
void UninstallHooks();

class HookException : public exception {
	string msg;

public:
	HookException(const string& m="Hooks failed to install!") : msg(m){}
	const char* what() { return msg.c_str(); }
};