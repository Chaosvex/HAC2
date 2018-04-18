#include "Preferences.h"
#include "misc.h"
#include <fstream>
#include <sstream>

namespace Preferences {

bool load();
void parse(std::string line);
std::map<std::string, std::string> prefs;

void initialise() {
	if(prefs.empty()) {
		load();
	}
}

void save() {
	std::string path = getWorkingDir() + "preferences.ini";
	std::ofstream ofs(path, std::ios::trunc);

	for(std::map<std::string, std::string>::iterator iter = prefs.begin(); iter != prefs.end(); ++iter) {
		ofs << iter->first << "=" << iter->second << std::endl;
	}

	ofs.close();
}

bool load() {
	std::string path = getWorkingDir() + "preferences.ini";
	std::ifstream ifs(path);

	std::string line;

	while(getline(ifs, line)) {
		parse(line);
	}

	ifs.close();
	return ifs.good();
}

void parse(std::string line) {
	size_t pos = line.find("=");
	if(pos != std::string::npos) {
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);
		boost::trim(key);
		boost::trim(value);
		Preferences::prefs[key] = value;
	}
}

}