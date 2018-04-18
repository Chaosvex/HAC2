#include "OpticLua.h"
#include "OpticAPI.h"
#include "Events.h"
#include "lua.hpp"
#include <Windows.h>

#define LUA_ENUM(L, name, val) \
  lua_pushlstring(L, #name, sizeof(#name)-1); \
  lua_pushnumber(L, val); \
  lua_settable(L, -3);

namespace Optic {

OpticLua::LuaGlobal::LuaGlobal(lua_State* state, const std::string& global) : lstate(state) {
	lua_getglobal(state, global.c_str());
}

OpticLua::LuaGlobal::~LuaGlobal() {
	lua_pop(lstate, 1);
}

void OpticLua::registerFunctions() {
	API::registerFunctions(luaState);
}

OpticLua::OpticLua(std::string lua) {
	luaState = luaL_newstate();
    luaL_openlibs(luaState);

	if(luaL_dostring(luaState, lua.c_str()) == 1) {
		std::string error = lua_tostring(luaState, -1);
		lua_pop(luaState, 1);
		throw OpticLuaException(error);
	}

	registerFunctions();
}

void OpticLua::call(const std::string& function, std::shared_ptr<Event> event) {
	int numargs = 0;
	lua_getglobal(luaState, function.c_str());

	if(event->type == PLAYER_KILLS) {
		PlayerKilled* levent = reinterpret_cast<PlayerKilled*>(event.get());
		lua_pushinteger(luaState, levent->event_id);
		lua_pushinteger(luaState, levent->killer);
		lua_pushinteger(luaState, levent->victim);
		lua_pushinteger(luaState, levent->local);
		lua_pushnumber(luaState, levent->time);
		numargs = 5;
	} else if(event->type == PLAYER_ENVIRONMENTAL_DEATHS) {
		PlayerEnvironmentalDeath* levent = reinterpret_cast<PlayerEnvironmentalDeath*>(event.get());
		lua_pushinteger(luaState, levent->event_id);
		lua_pushinteger(luaState, levent->victim);
		lua_pushinteger(luaState, levent->local);
		lua_pushnumber(luaState, levent->time);
		numargs = 4;
	} else if(event->type == CTF_EVENT) {
		CTFEvent* levent = reinterpret_cast<CTFEvent*>(event.get());
		lua_pushinteger(luaState, levent->event_id);
		lua_pushinteger(luaState, levent->killer);
		lua_pushinteger(luaState, levent->killed);
		lua_pushinteger(luaState, levent->local);
		lua_pushnumber(luaState, levent->time);
		numargs = 5;
	}

	if(lua_pcall(luaState, numargs, 0, 0) != 0) {
		std::string error = lua_tostring(luaState, -1);
		lua_pop(luaState, 1);
		throw OpticLuaException(error);
	}
}

std::string OpticLua::getVersion() {
	LuaGlobal global(luaState, "api_version");
	
	if(lua_isstring(luaState, 1)) {
		return std::string(lua_tostring(luaState, -1));
	} else {
		throw OpticLuaException("api_version not defined!");
	}
}

OpticLua::~OpticLua() {
	lua_close(luaState);
}

void OpticLua::registerGlobal(const std::string& key, void* global) {
	lua_pushlightuserdata(luaState, global);
	lua_setglobal(luaState, key.c_str());
}

void OpticLua::registerCallbacks(OpticEventHandler* handler) {
	createEventTable();
	lua_pushlightuserdata(luaState, handler);
	lua_setglobal(luaState, "handler");
	lua_getglobal(luaState, "register_callbacks");
	if(lua_pcall(luaState, 0, 0, 0) != 0) {
		std::string error = lua_tostring(luaState, -1);
		lua_pop(luaState, 1);
		throw OpticLuaException(error);
	}
}

void OpticLua::createEventTable() {
	lua_newtable(luaState);
	int i = 1;
	LUA_ENUM(luaState, RESOLUTION_CHANGE, i++);
    LUA_ENUM(luaState, POST_MAP_LOAD, i++);
	LUA_ENUM(luaState, QUEUE_JOIN, i++);
	LUA_ENUM(luaState, QUEUE_LEAVE, i++);
	LUA_ENUM(luaState, HUD_RECOLOURED, i++);
	LUA_ENUM(luaState, FOV_ADJUSTED, i++);
	LUA_ENUM(luaState, PLAYER_KILLS, i++);
	LUA_ENUM(luaState, PLAYER_ENVIRONMENTAL_DEATHS, i++);
	LUA_ENUM(luaState, CTF_EVENTS, i++);
	LUA_ENUM(luaState, GAME_OVER, i++);
	LUA_ENUM(luaState, PCR_SHOW, i++);
	LUA_ENUM(luaState, PCR_EXIT_SHOW, i++);
	LUA_ENUM(luaState, HIT_DETECT, i++);
	lua_setglobal(luaState, "cb");
}

};