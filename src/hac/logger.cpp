#include "logger.h"

std::fstream Logger::log;
Logger* Logger::logger;
bool Logger::outputEnabled;
const char* Logger::logname;
Logger::MESSAGE_TYPE Logger::outputLevel;

Logger::Logger(const char* filename, MESSAGE_TYPE level, bool enabled) {
	setOutputLevel(level);
	setOutput(enabled);
	Logger::logname = filename;
}

Logger* Logger::getInstance() {
	if(logger == NULL) {
		logger = new Logger("log.txt", ERROR, false);
	}

	return logger;
}

bool Logger::init() {
	log.open(logname, fstream::out | fstream::app);
	return (log != NULL);
}

void Logger::setLogFile(const char* filename) {
	logname = filename;
}

/*
 * Hacky
 */
string Logger::getPath() {
	char path[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, path);
	string fullPath(path);
	fullPath += "\\My Games\\Halo CE\\hac\\";
	CreateDirectory(fullPath.c_str(), NULL);
#ifdef _DEBUG
	fullPath += "logd.txt";
#else
	fullPath += "log.txt";
#endif
	OutputDebugString(fullPath.c_str());
	return fullPath;
}

void Logger::setOutput(bool enabled) {
	outputEnabled = enabled;
}

void Logger::write(std::string text, MESSAGE_TYPE type) {
	string message = generate(text, type);
	log << message.c_str() << flush;
	if(type >= outputLevel && outputEnabled) {
		if(type == ERROR) {
			cerr << "\r" << text.c_str() << endl;
		} else {
			cout << "\r" << text.c_str() << endl;
		}
		reprompt();
	}
}

void Logger::writeln(std::string text, MESSAGE_TYPE type) {
	string message = generate(text, type);
	log << message.c_str() << endl;
	if(type >= outputLevel && outputEnabled) {
		if(type == ERROR) {
			cerr << "\r" << text.c_str() << endl;
		} else {
			cout << "\r" << text.c_str() << endl;
		}
		reprompt();
	}
}

string Logger::generate(std::string text, MESSAGE_TYPE type) {
	string message;

	switch(type) {
		case ERROR:
			message += "[ERROR] ";
			break;
		case NOTICE:
			message += "[NOTICE] ";
			break;
		case MESSAGE:
			message += "[MESSAGE] ";
			break;
	}

	message += text;
	return message;
}

void Logger::setOutputLevel(MESSAGE_TYPE level) {
	outputLevel = level;
}

void Logger::reprompt() {
	cout << "hac( " << flush;
}