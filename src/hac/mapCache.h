#pragma once

#include <string>
#include <map>
#include <cstdint>

bool flushCache();
extern std::map<std::string, std::uint32_t> maps; //remove in future
void alternateLoad();