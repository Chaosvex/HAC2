#include "ArgContainer.h"

ArgContainer::ArgContainer(const std::string &arg) {
	this->arg_line = arg;
	argc = 0;
	unsigned int word_start = 0;
	size_t closing;

	while(true) {
		if(arg[word_start] == '"') {
			word_start++;
			closing = arg.find_first_of('"', word_start);
			if(closing == -1) {
				closing = arg.length();
			}
		} else if(arg[word_start] == '\'') {
			word_start++;
			closing = arg.find_first_of('\'', word_start);
			if(closing == -1) {
				closing = arg.length();
			}
		} else {
			closing = arg.find_first_of(' ', word_start);
			if(closing == -1) {
				closing = arg.length();
			}
		}

		std::string word = arg.substr(word_start, closing - word_start);
		args.emplace_back(word);
		argc++;
		word_start = arg.find_first_not_of(' ', closing + 1);

		if(word_start == -1) {
			break;
		}

		if((word_start + 2 <= arg.length())) {
			if(arg[word_start+1] == '"' || arg[word_start+1] == '\'')
			word_start++;
		}
	}

}

std::string ArgContainer:: operator[](size_t i) {
	return args.at(i);
}

std::string ArgContainer::str(bool orginal) {
	 if(orginal) {
		return arg_line;
	 }

	std::string ret;

	for(size_t i = 0; i < args.size(); i++) {
		ret += args[i];
		if(i != (args.size() - 1)) {
			ret += " ";
		}
	}

	return ret;
}

bool ArgContainer::ProcessBoolValue() {
	if(this->operator [](1) == "1" || this->operator [](1) == "true" || this->operator [](1) == "on" || this->operator [](1) == "enabled") return true;
	if(this->operator [](1) == "0" || this->operator [](1) == "false" || this->operator [](1) == "off" || this->operator [](1) == "disabled") return false;
	return false;
}