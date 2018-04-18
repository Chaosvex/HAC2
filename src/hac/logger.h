#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <shlobj.h>

using namespace std;

class Logger {
public:
	enum MESSAGE_TYPE {
		MESSAGE, NOTICE, ERROR
	};

	bool init();
	void write(string text, MESSAGE_TYPE type);
	void writeln(string text, MESSAGE_TYPE type);
	void setOutputLevel(MESSAGE_TYPE level);
	void reprompt();
	void setOutput(bool enabled);
	void setLogFile(const char* filename);
	static Logger* getInstance();


private:
	Logger(const char* filename, MESSAGE_TYPE level, bool enabled);
	static Logger* logger;
	static const char* logname;
	static fstream log;
	static MESSAGE_TYPE type;
	string generate(string text, MESSAGE_TYPE type);
	static MESSAGE_TYPE outputLevel;
	static bool outputEnabled;
	static string getPath();
};

#endif