#include "QueryActions.h"
#include "QueryParser.h"
#include "AppSearch.h"
#include "EngineState.h"
#include "EngineFunctions.h"
#include "EngineTypes.h"
#include "EnginePointers.h"
#include <string>
#include <sstream>

namespace es = EngineState;
namespace et = EngineTypes;

namespace {
	void voipDetect(const char* response);
	void hudGreeting(const char* response);
}

void queryActions(const char* response) {
	voipDetect(response);
	hudGreeting(response);
}

namespace {

void hudGreeting(const char* response) {
	QueryParser parser(response);
	std::string message = parser.getValue("welcome");
	
	if(!message.empty()) {
		HUDMessage(message);
	}
}

void voipDetect(const char* response) {
	QueryParser parser(response);
	std::string ts = parser.getValue("ts3");
	std::string vent = parser.getValue("vent3");
	std::wstring wprof = (*pPlayerHeader)->firstPlayer->name;
	std::string profile(wprof.begin(), wprof.end());
	std::string uri;

	if(!ts.empty() && AppSearch::find("TeamSpeak 3 Client")) {
		std::string password = parser.getValue("ts3_pass");
		std::string channel = parser.getValue("ts3_chan");
		std::string login = parser.getValue("ts3_login");
		std::string subchan = parser.getValue("ts3_subchan");
		std::string chanpass = parser.getValue("ts3_chanpass");

		std::stringstream stream;
		stream << "ts3server://" << ts << "?nickname=" << profile << "?channel=" << channel
				<< "?password=" << password << "?subchannel=" << subchan << "?channelpassword="
				<< chanpass << "?loginname=" << login;
		uri = stream.str();
	}

	if(!vent.empty() && (AppSearch::find("Ventrilo Client") || AppSearch::find("Ventrilo Client for Windows x64"))) {
		std::string serverName = parser.getValue("vent3_sname");
		std::string password = parser.getValue("vent3_pass");
		std::string channel = parser.getValue("vent3_chan");
		std::string chanpass = parser.getValue("vent3_chanpass");

		std::stringstream stream;
		stream << "ventrilo://" << vent << "&servername=" << serverName << "&serverpassword=" << password
				<< "&channelname=" << channel << "&channelpassword=" << chanpass;
		uri = stream.str();
	}
	
	if(!uri.empty()) {
		es::setState(es::VOIP_AVAILABLE);
	} else {
		es::clearState(es::VOIP_AVAILABLE);
	}
}

}