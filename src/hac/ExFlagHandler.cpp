#include "ExFlagHandler.h"
#include "EngineFunctions.h"
#include "EngineState.h"
#include "QueryParser.h"
#include <string>

ExFlagHandler::ExFlagHandler(unsigned int flags) {
	this->flags = flags;
}

void ExFlagHandler::reset() {
	ToggleMTV(false);
	clearState(EngineState::MULTITEAM_VEHICLES);
	clearState(EngineState::SCRIM_MODE);
}

void ExFlagHandler::set() {
	process();
}

void ExFlagHandler::process() {
	reset();

	//Multi-team vehicles
	bool toggle = (MULTITEAM_VEHICLES & flags) != 0;
	ToggleMTV(toggle);
	setState(EngineState::MULTITEAM_VEHICLES);

	//Scrim mode
	if((SCRIM_MODE & flags) != 0) {
		setState(EngineState::SCRIM_MODE);
	}
}

void setExtendedFlags(const char* response) {
	QueryParser parser(response);
	unsigned int flags = 0;
	std::string flagstr = parser.getValue("sapp_flags");

	if(!flagstr.empty()) {
		flags = atoi(flagstr.c_str());
		if(errno == ERANGE) {
			flags = 0;
		}
	}

	ExFlagHandler handler(flags);
	handler.set();
}