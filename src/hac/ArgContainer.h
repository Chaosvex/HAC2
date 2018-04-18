#pragma once

#include <vector>
#include <string>

class ArgContainer {
	std::vector <std::string> args;
	std::string arg_line;

public:
	size_t argc;
	ArgContainer(const std::string &arg);
	std::string operator[](size_t i);
	std::string str(bool orginal=1);
	bool ProcessBoolValue();
};