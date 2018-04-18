#include "PowerupTimer.h"
#include "EngineFunctions.h"
#include "EngineTypes.h"

using namespace EngineTypes;

PowerupTimer::PowerupTimer(std::vector<Timings> timings, bool broadcast) 
	: timings(timings), stopping(false), elapsed(0), chatBroadcast(broadcast),
	TICK_INTERVAL(500), BEGIN_OFFSET(700) {
	initialise();
}

void __cdecl PowerupTimer::startTimer(void* timer) {
	PowerupTimer *t = static_cast<PowerupTimer*>(timer);
	t->run();
}

void PowerupTimer::run() {
	while(!stopping) {
		QueryPerformanceCounter(&begin);
		decrementTime();
		Sleep(TICK_INTERVAL);
		QueryPerformanceCounter(&end);
		elapsed = (end.QuadPart - begin.QuadPart) / frequency;
	}
}

void PowerupTimer::decrementTime() {
	for(size_t i = 0, j = timings.size(); i < j; i++) {
		timings[i].countdown -= elapsed;
		calculateWarnings(timings[i]);

		if(timings[i].countdown <= 0) {
			timings[i].countdown = timings[i].frequency + timings[i].countdown;
			timings[i].signalled = 0;
		}
	}
}

void PowerupTimer::calculateWarnings(Timings& object) {
	double seconds = object.countdown / 1000;
	int warnTime = 0;
	bool warn = true;
	
	if(object.signalled == 0 && seconds <= 30 && seconds > 15) {
		warnTime = 30;
	} else if(object.signalled == 1 && seconds <= 15 && seconds > 5) {
		warnTime = 15;
	} else if(object.signalled == 2 && seconds <= 5 && seconds > 1) {
		warnTime = 5;
	} else if(object.signalled == 3 && seconds <= 0) {
		warnTime = 0; //uh
	} else {
		warn = false;
	}

	if(warn) {
		displayWarning(object.description, warnTime);
		object.signalled++;
	}
}

void PowerupTimer::displayWarning(const std::string& description, int time) {
	WCHAR buffer[64];

	if(time > 0) {
		swprintf_s(buffer, 64, L"%hs spawning in %d seconds!", description.c_str(), time);
	} else {
		swprintf_s(buffer, 64, L"%hs has spawned!", description.c_str());
		PlayMPSound(_multiplayer_sound_countdown_timer_end);
	}

	HUDMessage(buffer);
	
	if(chatBroadcast) { 
		ChatMessage(buffer, CHAT_TEAM);
	}
}

void PowerupTimer::stop() {
	stopping = true;
}

void PowerupTimer::broadcast(bool setting) {
	chatBroadcast = setting;
}

void PowerupTimer::initialise() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
    frequency = freq.QuadPart / 1000.0;

	/* An offset is substracted from the starting times to compensate for the
	   time spent loading the map. The timer only starts once the map has finished
	   loading which means it could be slightly off at the start. Perhaps use the
	   query hook. */
	for(auto i = timings.begin(); i != timings.end(); i++) {
		(*i).countdown = (*i).frequency - BEGIN_OFFSET;
	}
}