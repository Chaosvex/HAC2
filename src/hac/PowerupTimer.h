#pragma once

#include <vector>
#include <string>
#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct Timings {
	Timings() : frequency(0), countdown(0), signalled(0) {}
	double countdown;
	std::uint32_t frequency;
	std::string description;
	std::uint8_t signalled;
};

class PowerupTimer {
	const std::uint16_t TICK_INTERVAL;
	const std::uint16_t BEGIN_OFFSET;
	std::vector<Timings> timings;
	void run();
	bool stopping;
	void initialise();
	double frequency;
	LARGE_INTEGER begin, end;
	void decrementTime();
	void calculateWarnings(Timings& object);
	void displayWarning(const std::string& description, int time);
	double elapsed;
	bool chatBroadcast;
	PowerupTimer(const PowerupTimer&);
	PowerupTimer& operator=(const PowerupTimer&);

public:
	PowerupTimer(const std::vector<Timings> timings, bool broadcast = false);
	static void __cdecl startTimer(void* timer);
	void stop();
	void broadcast(bool enable);
};