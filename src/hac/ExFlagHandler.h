#pragma once

class ExFlagHandler {
	enum {
		NO_LEAD = 1,
		SCRIM_MODE = 2,
		SAPP_MAPCYCLE = 4,
		HALORANK = 8,
		ANTICHEAT = 16,
		MULTITEAM_VEHICLES = 32
	};

	unsigned int flags;
	void process();

public:
	ExFlagHandler(unsigned int flags);
	void set();
	void reset();
};

void setExtendedFlags(const char* response);