#pragma once

#include <string>
#include <cstdint>

enum EVENTS : unsigned int {
	LUA_API_SAFE_BEGIN, //this might be an abuse of enums
		RESOLUTION_CHANGE,
		POST_MAP_LOAD,
		QUEUE_JOIN,
		QUEUE_LEAVE,
		HUD_RECOLOURED,
		FOV_ADJUSTED,
		PLAYER_KILLS,
		PLAYER_ENVIRONMENTAL_DEATHS,
		CTF_EVENT,
		GAME_OVER,
		PCR_SHOW,
		PCR_EXIT_SHOW,
		HIT_DETECT,
	LUA_API_SAFE_END,
	
	THREAD_EXIT,
	CONNECTION_LOST,
	CONNECTION_ESTABLISHED,
	NO_CONNECTION,
	LOGIN_ATTEMPT,
	LOGIN_SUCCESS,
	LOGIN_FAIL,
	ALREADY_LOGGED_IN,
	NOT_LOGGED_IN,
	INSUFFICIENT_RIGHTS,
	USER_LOGGED_IN,
	USER_LOGGED_OUT,
	USER_JOIN_GAME,
	USER_LEAVE_GAME,
	USER_IS_CHEATING,
	OPTIC_SOUND_TRIGGER,
	PING_ANTICHEAT_THREAD,
	PONG_ANTICHEAT_THREAD,
	REQUEST_ANTICHEAT_THREAD_HANDLE,
	ANTICHEAT_THREAD_HANDLE,
	LAST //this too
};

struct Event {
	Event(EVENTS type) : type(type) {};
	EVENTS type;
};

struct LoginEvent : Event {
	LoginEvent(std::string username, std::string password)
	 : Event(LOGIN_ATTEMPT), username(username), password(password) {}; 
	std::string username;
	std::string password;
};

struct PlayerKilled : Event {
	PlayerKilled(std::uint32_t event_id, std::uint32_t victim, std::uint32_t killer, std::uint32_t local, std::uint32_t time)
		: Event(PLAYER_KILLS), event_id(event_id), victim(victim), killer(killer), local(local), time(time) {};
	std::uint32_t event_id, victim, killer, local;
	std::uint32_t time;
};

struct PlayerEnvironmentalDeath : Event {
	PlayerEnvironmentalDeath(std::uint32_t event_id, std::uint32_t victim, std::uint32_t local, std::uint32_t time)
		: Event(PLAYER_ENVIRONMENTAL_DEATHS), event_id(event_id), victim(victim), local(local), time(time) {};
	std::uint32_t event_id, victim, local;
	std::uint32_t time;
};

struct CTFEvent : Event {
	CTFEvent(std::uint32_t event_id, std::uint32_t killed, std::uint32_t killer, std::uint32_t local, std::uint32_t time)
		: Event(CTF_EVENT), event_id(event_id), killed(killed), killer(killer), local(local), time(time) {};
	std::uint32_t event_id, killed, killer, local;
	std::uint32_t time;
};

struct GameEvent : Event {

};

struct CheatEvent : Event {

};

struct ThreadHandleEvent : Event {
	ThreadHandleEvent(void* thread) : Event(ANTICHEAT_THREAD_HANDLE), acthread(thread) {}; 
	void* acthread;	
};

struct OpticSoundEvent : Event {
	OpticSoundEvent(std::string sound) : Event(OPTIC_SOUND_TRIGGER), file(sound) {}; 
	std::string file;	
};