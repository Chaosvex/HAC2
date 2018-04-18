#include "TimerManager.h"
#include "EnginePointers.h"
#include "resource.h"
#include "PowerupTimer.h"
#include "rapidxml\rapidxml.hpp"
#include "console.h"
#include <string>
#include <map>
#include <Windows.h>
#include <process.h>

using namespace rapidxml;

void parseEntry(xml_node<>* entry);
void parseObject(std::vector<Timings>& objects, xml_node<>* object);
std::map<std::string, std::vector<Timings>> timers;
bool initialise();
bool timersEnabled = false;
PowerupTimer *timer;
HANDLE thread;
bool broadcast = false;

void enableTimers(bool silent) {
	if(!timersEnabled) {
		if(initialise()) {
			timersEnabled = true;
			hkDrawText("Power-up timers enabled!", C_TEXT_GREEN);
		} else {
			hkDrawText("Unable to enable power-up timers!", C_TEXT_RED);
		}
	} else if(!silent) {
		hkDrawText("Power-up timers are already enabled!", C_TEXT_BLUE);
	}
}

void timerBroadcast(bool enable) {
	broadcast = enable;
	if(timer != NULL) {
		timer->broadcast(enable);
	}
}

void disableTimers() {
	endTimer(true);
	timers.clear();
	timersEnabled = false;
	hkDrawText("Timers disabled.", C_TEXT_YELLOW);
}

void resetTimer() {
	if(timer != NULL) {
		endTimer(true);
		beginTimerAuto();
		hkDrawText("Timer reset!", C_TEXT_YELLOW);
	} else {
		hkDrawText("No active timer! Use begin_timer to start a new timer.", C_TEXT_RED);
	}
}

void beginTimerUser() {
	enableTimers(true);

	if(timersEnabled) {
		if(timer == NULL) {
			try {
				timer = new PowerupTimer(timers.at(pCurrentMap), broadcast);
				thread = (HANDLE)_beginthread(&PowerupTimer::startTimer, 0, (void*)timer);
				hkDrawText("Timers started! Good luck, have fun.", C_TEXT_GREEN);
			} catch(std::out_of_range) {
				hkDrawText("Timers are not available for this map.", C_TEXT_RED);
			}
		} else {
			hkDrawText("Timing is already in progress!", C_TEXT_RED);
		}
	}
}

void beginTimerAuto() {
	while(timer != NULL) endTimer(true);

	if(timersEnabled && strcmp(pCurrentMap, "ui") != 0) {
		try {
			timer = new PowerupTimer(timers.at(pCurrentMap), broadcast);
			_beginthread(&PowerupTimer::startTimer, 0, (void*)timer);
		} catch(std::out_of_range) {
			hkDrawText("Timers are not available for this map.", C_TEXT_RED);
		}
	}
}

/*Fix to join thread*/
void endTimer(bool silent) {
	if(timer != NULL) {
		timer->stop();
		WaitForSingleObject(thread, INFINITE);
		delete timer;
		timer = NULL;

		if(!silent) {
			hkDrawText("Timing stopped.", C_TEXT_BLUE);
		}

	} else if(!silent) {
		hkDrawText("No active timer!", C_TEXT_RED);
	}
}

bool initialise() {
	bool success = true;
	HMODULE handle = GetModuleHandle("hac.dll");

	//Locate the resource
	HRSRC fRes = FindResource(handle, MAKEINTRESOURCE(IDR_XML2), "XML");
	if(fRes == NULL) {
		return false;
	}

	//Load the resource
	HGLOBAL fGlob = LoadResource(handle, fRes);
	if(fGlob == NULL) {
		return false;
	}

	//Fetch pointer to resource
	char* timings = (char*)LockResource(fGlob);
	size_t length = SizeofResource(handle, fRes);

	if(timings == NULL) {
		return false;
	}

	//RapidXML modifies the XML string in place, so it needs copied
	char* xml = new char[length + 1];
	strncpy_s(xml, length + 1, timings, length);

	try {
		xml_document<> doc;    // character type defaults to char
		doc.parse<0>(xml);    // 0 means default parse flags
		xml_node<> *node = doc.first_node("maps");
		
		//Loop over all of the maps
		for(xml_node<> *n = node->first_node(); n != 0; n = n->next_sibling()) {
			xml_attribute<> *a = n->first_attribute("name");
			if(a != NULL) {
				parseEntry(n);
			}
		}
	} catch(rapidxml::parse_error& e) {
		OutputDebugString(e.what());
		success = false;
	}

	delete[] xml;
	return success;
}

void parseEntry(xml_node<>* entry) {
	std::vector<Timings> objects;
	xml_attribute<> *name = entry->first_attribute("name");

	//Iterate over the object nodes
	for(xml_node<> *object = entry->first_node(); object != 0; object = object->next_sibling()) {
		if(strcmp(object->name(), "object") == 0) {
			parseObject(objects, object);
		}
	}

	timers[name->value()] = objects;
}

void parseObject(std::vector<Timings>& objects, xml_node<>* object) {
	Timings timer;
	bool foundType = false;

	for(xml_node<> *details = object->first_node(); details != 0; details = details->next_sibling()) {
		if(strcmp(details->name(), "type") == 0) {
			timer.description = std::string(details->value());
			foundType = true;
		} else if(strcmp(details->name(), "frequency") == 0) {
			timer.frequency = atoi(details->value());
		}
	}

	if(foundType && timer.frequency > 0) {
		objects.emplace_back(timer);
	}
}