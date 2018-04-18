#include "versionchanger.h"
#include "EnginePointers.h"
#include "QueryParser.h"
#include <string>
#include <cstdint>

std::uint32_t versionNum;

namespace {
	std::uint32_t lookup(const std::string& version);
}

void versionCheck(const char* queryResponse) {
	QueryParser parser(queryResponse);
	std::string versionStr = parser.getValue("gamever");
	DWORD version = lookup(versionStr);

	if(version != 0 && versionNum != NULL) {
		memcpy((void*)versionNum, &version, 4);
	}
}

namespace {

uint32_t lookup(const std::string& version) {
	uint32_t versionNum = 0;
	std::string game(productType);

	if(game == "halom") { //HCE
		if(version == "01.00.10.0621") {
			versionNum = 0x00096A27;
		} else if(version == "01.00.09.0620") {
			versionNum = 0x00096A27;
		} else if(version == "01.00.08.0616") {
			versionNum = 0x005BCFE7;
		} else if(version == "01.00.07.0613") {
			versionNum = 0x005BC42F;
		} else if(version == "01.00.00.0609") {
			versionNum = 0x00094ECF;
		}
	} else if(game == "halor") { //HPC
		if(version == "01.00.10.0621") {
			versionNum = 0x00096640;
		} else if(version == "01.00.09.0620") {
			versionNum = 0x00096640;
		} else if(version == "01.00.08.0616") {
			versionNum = 0x00096640;
		} else if(version == "01.00.07.0613") {
			versionNum = 0x000956A0;
		} else if(version == "01.00.06.0612") {
			versionNum = 0x000956A0;
		} else if(version == "01.00.04.0607") {
			versionNum = 0x00094318;
		} else if(version == "01.00.03.0605") {
			versionNum = 0x00093B48;
		} else if(version == "01.00.02.0581") {
			versionNum = 0x0008DD88;
		} else if(version == "01.00.01.0580") {
			versionNum = 0x0008D9A0;
		} else if(version == "01.00.00.0564") {
			versionNum = 0x00000006;
		}
	}

	return versionNum;
}

}
