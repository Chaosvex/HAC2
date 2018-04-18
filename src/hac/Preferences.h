#pragma once

#include "console.h"
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <boost\lexical_cast.hpp>
#include <boost\algorithm\string.hpp>

namespace Preferences {

extern std::map<std::string, std::string> prefs;
void initialise();
void save();

template<typename T>
void add(const std::string& key, T value) {
	std::stringstream val;
	val << value;
	prefs[key] = val.str();
}

template<typename T>
T find(const std::string& key, T default) {
	if(prefs.find(key) != prefs.end()) {
		try {
			std::string value = prefs[key];
			boost::trim(value);
			if(value == "true") { //Boost can't handle true/false string to bool. /golfclap
				value = "1";
			} else if(value == "false") {
				value = "0";
			}
			return boost::lexical_cast<T>(value);
		} catch(boost::bad_lexical_cast&) {
			std::stringstream ss;
			ss << "An error occured while processing preference (" << key << " => " << prefs[key] << ")!";
			hkDrawText(ss.str(), C_TEXT_RED);
		}
	}
		
	return default;
}

}