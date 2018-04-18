#pragma once

#include "resource.h"
#include <string>

namespace Language {
	std::string GetString(unsigned int identifier);
	std::wstring GetWideString(unsigned int identifier);
};