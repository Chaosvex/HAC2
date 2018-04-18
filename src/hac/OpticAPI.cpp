#include "OpticAPI.h"
#include "OpticLua.h"
#include "OpticPack.h"
#include "OpticRender.h"
#include "OpticAnimation.h"
#include "EngineFunctions.h"
#include "EngineTypes.h"
#include "EnginePointers.h"
#include "FoVControl.h"
#include "console.h"
#include "OpticEventHandler.h"
#include "OpticHooks.h"
#include "DebugHelper.h"
#include "lua.hpp"
#include <sstream>

#pragma warning(disable : 4800)

namespace Optic { namespace API {

int attach_sprite(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 2) {
		luaL_error(state, "Invalid number of arguments");
	}

	lua_getglobal(state, "renderer");
	OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -1));
	lua_pop(state, 1);

	if(!lua_isstring(state, 1)) {
		return luaL_argerror(state, 1, "medal name expected");
	}

	if(!lua_isstring(state, 2)) {
		return luaL_argerror(state, 2, "sprite name expected");
	}

	try {
		render->attachSprite(lua_tostring(state, 1), lua_tostring(state, 2));
	} catch(std::runtime_error& e) {
		return luaL_error(state, e.what());
	}

	return 0;
}

int create_sprite(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 2 && args != 3) {
		luaL_error(state, "Invalid number of arguments");
	}

	if(!lua_isstring(state, 1)) {
		return luaL_argerror(state, 1, "sprite name expected");
	}

	if(!lua_isstring(state, 2)) {
		return luaL_argerror(state, 2, "image name expected");
	}

	if(args == 3 && !lua_isstring(state, 3)) {
		return luaL_argerror(state, 3, "animation name expected");
	}

	lua_getglobal(state, "renderer");
	lua_getglobal(state, "resources");
	OpticPack* pack = static_cast<OpticPack*>(lua_touserdata(state, -1));
	OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -2));
	lua_pop(state, 2);

	try {
		OpticImage image = pack->fetchImage(lua_tostring(state, 2));
		if(args == 3) {
			render->createSprite(lua_tostring(state, 1), image, lua_tostring(state, 3));
		} else {
			render->createSprite(lua_tostring(state, 1), image);
		}
	} catch(OpticException& e) {
		return luaL_error(state, e.what());
	}

	return 0;
}

int create_animation(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 1) {
		luaL_error(state, "Invalid number of arguments");
	}

	if(!lua_isstring(state, 1)) {
		return luaL_argerror(state, 1, "animation name expected");
	}

	lua_getglobal(state, "renderer");
	OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -1));
	lua_pop(state, 1);

	try {
		render->createAnimation(lua_tostring(state, 1));
	} catch(OpticException& e) {
		return luaL_error(state, e.what());
	}

	return 0;
}

int create_medal(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 1 && args != 2) {
		luaL_error(state, "Invalid number of arguments");
	}

	if(!lua_isstring(state, 1)) {
		return luaL_argerror(state, 1, "medal name expected");
	}

	if(args == 2 && !lua_isstring(state, 2)) {
		return luaL_argerror(state, 2, "animation name expected");
	}

	lua_getglobal(state, "renderer");
	OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -1));
	lua_pop(state, 1);

	try {
		if(args == 1) {
			render->createMedal(lua_tostring(state, 1));
		} else {
			render->createMedal(lua_tostring(state, 1), lua_tostring(state, 2));
		}
	} catch(OpticException& e) {
		return luaL_error(state, e.what());
	}

	return 0;
}

int sprite_properties(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 7) {
		luaL_error(state, "Invalid number of arguments");
	}

	if(!lua_isstring(state, 1)) {
		return luaL_argerror(state, 1, "sprite name expected");
	}

	if(!lua_isboolean(state, 2)) {
		return luaL_argerror(state, 2, "boolean expected");
	}

	for(int i = 3; i != 7; i++) {
		if(!lua_isnumber(state, i)) {
			return luaL_argerror(state, i, "number expected");
		}
	}

	lua_getglobal(state, "renderer");
	OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -1));
	lua_pop(state, 1);

	try {
		render->setSpriteProperties(lua_tostring(state, 1), lua_toboolean(state, 2), lua_tointeger(state, 3),
									lua_tointeger(state, 4),lua_tointeger(state, 5), lua_tointeger(state, 6),
									lua_tointeger(state, 7));
	} catch(OpticException& e) {
		return luaL_error(state, e.what());
	}

	return 0;
}

int add_keyframe(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 4 && args != 5 && args != 8) {
		luaL_error(state, "Invalid number of arguments for adding keyframes");
	}

	if(!lua_isstring(state, 1)) {
		return luaL_argerror(state, 1, "animation name expected");
	}

	if(!lua_isnumber(state, 2)) {
		return luaL_argerror(state, 2, "time value expected");
	}

	if(!lua_isnumber(state, 3)) {
		return luaL_argerror(state, 3, "transition state expected");
	}

	if(!lua_isnumber(state, 4)) {
		return luaL_argerror(state, 4, "attribute expected");
	}

	if(args == 5 && !lua_isnumber(state, 5)) {
		return luaL_argerror(state, 5, "tween type expected");
	}

	if(args == 8) {
		for(int i = 5; i != 8; i++) {
			if(!lua_isnumber(state, i)) {
				return luaL_argerror(state, i, "control point coordinate expected");
			}
		}
	}

	lua_getglobal(state, "renderer");
	OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -1));
	lua_pop(state, 1);

	try {
		if(args == 4) {
			render->addKeyframe(lua_tostring(state, 1), Keyframe(lua_tointeger(state, 2), lua_tonumber(state, 3)),
								TRANSFORM(lua_tointeger(state, 4)), LINEAR);	
		} else if(args == 5) {
			render->addKeyframe(lua_tostring(state, 1), Keyframe(lua_tointeger(state, 2), lua_tonumber(state, 3)),
								TRANSFORM(lua_tointeger(state, 4)), TWEEN(lua_tointeger(state, 5)));
		} else {
			OpticBezier bezier(lua_tonumber(state, 5), lua_tonumber(state, 6), lua_tonumber(state, 7), lua_tonumber(state, 8));
			render->addKeyframe(lua_tostring(state, 1), Keyframe(lua_tointeger(state, 2), lua_tonumber(state, 3)),
								TRANSFORM(lua_tointeger(state, 4)), bezier);
		}
	} catch(OpticException& e) {
		luaL_error(state, e.what());
	}

	return 0;
}

int in_game(lua_State* state) {
	lua_pushboolean(state, (*pPlayerHeader)->m_wIsInMainMenu);
	return 1;
}

int server_max_players(lua_State* state) {
	lua_pushinteger(state, (*pPlayerHeader)->m_wMaxSlots);
	return 1;
}

int server_current_players(lua_State* state) {
	lua_pushinteger(state, (*pPlayerHeader)->m_wSlotsTaken);
	return 1;
}

int hud_message(lua_State* state) {
	lua_getglobal(state, "renderer");
	OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -1));
	lua_pop(state, 1);
	
	for(int i = 1, j = lua_gettop(state); i <= j; i++) {
		render->displayText(lua_tostring(state, i));
	}

	return 0;
}

int draw_distance(lua_State* state) {
	lua_pushnumber(state, *pDrawDistance);
	return 1;
}

int field_of_view(lua_State* state) {
	lua_pushnumber(state, FoV::get());
	return 1;
}

int game_version(lua_State* state) {
	lua_pushinteger(state, versionNum);
	return 1;
}

int server_name(lua_State* state) {
	std::wstring wname(serverName);
	std::string name(wname.begin(), wname.end());
	lua_pushstring(state, name.c_str());
	return 1;
}

int server_address(lua_State* state) {
	lua_pushstring(state, serverAddress);
	return 1;
}

int product_type(lua_State* state) {
	lua_pushstring(state, productType);
	return 1;
}

int console_print(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 2) {
		return luaL_error(state, "Invalid number of arguments for console printing");
	}

	if(!lua_isnumber(state, 2)) {
		return luaL_argerror(state, 2, "Colour expected");
	}

	if(!lua_isstring(state, 1)) {
		return luaL_argerror(state, 1, "Message string expected");
	}

	try {
		hkDrawText(lua_tostring(state, 1), C_TEXT_COLOUR(lua_tointeger(state, 2)));
	} catch(std::runtime_error& e) {
		return luaL_error(state, e.what());
	}
	
	return 0;
}

int distance(lua_State* state) {
	int args = lua_gettop(state);

	if(args % 2 == 1) {
		luaL_error(state, "Invalid number of arguments for distance");
	}

	double distance = 0.0;

	for(int i = 1, j = args / 2; i < j; ++i) {
		distance += pow((lua_tonumber(state, i) + lua_tonumber(state, j + i)), 2);
	}

	lua_Number num = sqrt(distance);
	lua_pushnumber(state, num);

	return 1;
}

int current_map(lua_State* state) {
	lua_pushstring(state, pCurrentMap);
	return 1;
}

int resolution(lua_State* state) {
	lua_pushinteger(state, pResolution->width);
	lua_pushinteger(state, pResolution->height);
	return 2;
}

int register_callback(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 2) {
		luaL_error(state, "Invalid number of arguments for registering callback");
	}
	
	lua_getglobal(state, "handler");
	OpticEventHandler* handler = static_cast<OpticEventHandler*>(lua_touserdata(state, -1));
	EVENTS event = static_cast<EVENTS>(lua_tointeger(state, 1));
	
	if(!handler->registerLuaCallback(event, lua_tostring(state, 2))) {
		luaL_error(state, "Invalid callback event ID");
	}

	return 0;
}

int medal_properties(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 5) {
		luaL_error(state, "Invalid number of arguments for medal properties");
	}

	if(!lua_isstring(state, 1)) {
		luaL_argerror(state, 1, "Medal name was expected");
	}

	if(!lua_isnumber(state, 2)) {
		luaL_argerror(state, 2, "Group number was expected");
	}

	if(!lua_isnumber(state, 3)) {
		luaL_argerror(state, 3, "Priority value was expected");
	}

	if(!lua_isboolean(state, 4)) {
		luaL_argerror(state, 4, "Immunity value expected");
	}

	if(!lua_isnumber(state, 5)) {
		luaL_argerror(state, 5, "Slide reverse animation time expected");
	}

	try {
		lua_getglobal(state, "renderer");
		OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -2));
		lua_pop(state, 1);

		render->setMedalProperties(lua_tostring(state, 1), lua_tointeger(state, 2), lua_tointeger(state, 3),
								   lua_toboolean(state, 4), lua_tointeger(state, 5));
	} catch(OpticException& e) {
		luaL_error(state, e.what());
	}

	return 0;
}

int medal_slide_animation(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 2) {
		luaL_error(state, "Invalid number of arguments for medal animation");
	}

	if(!lua_isstring(state, 1)) {
		luaL_argerror(state, 1, "Medal name was expected");
	}

	if(!lua_isstring(state, 2)) {
		luaL_argerror(state, 2, "Animation name was expected");
	}

	try {
		lua_getglobal(state, "renderer");
		OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -1));
		lua_pop(state, 1);

		render->setSlideAnimation(lua_tostring(state, 1), lua_tostring(state, 2));
	} catch(OpticException& e) {
		luaL_error(state, e.what());
	}

	return 0;
}

int display_medal(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 1 && args != 2) {
		luaL_error(state, "Invalid number of arguments for medal display");
	}

	if(!lua_isstring(state, 1)) {
		luaL_argerror(state, 2, "Either a medal name or image name was expected");
	}

	if(args == 2 && !lua_isstring(state, 2)) {
		luaL_argerror(state, 4, "An animation name was expected");
	}

	try {
		lua_getglobal(state, "renderer");
		lua_getglobal(state, "resources");
		
		OpticPack* pack = static_cast<OpticPack*>(lua_touserdata(state, -1));
		OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -2));
		lua_pop(state, 2);

		if(args == 2) {
			render->renderMedal(pack->fetchImage(lua_tostring(state, 1)), lua_tostring(state, 2));
		} else {
			std::string name = lua_tostring(state, 1);
			if(render->medalExists(name)) {
				render->renderMedal(name);
			} else {
				render->renderMedal(pack->fetchImage(name));
			}
		}
	} catch(OpticException& e) {
		luaL_error(state, e.what());
	}

	return 0;
}

int queue_audio(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 1 && args != 2) {
		luaL_error(state, "Invalid number of arguments for queue audio");
	}

	if(!lua_isstring(state, 1)) {
		luaL_argerror(state, 1, "Audio filename was expected");
	}

	if(args == 2 && !lua_isboolean(state, 2)) {
		luaL_argerror(state, 2, "Boolean indicating priority expected");
	}

	try {
		lua_getglobal(state, "renderer");
		lua_getglobal(state, "resources");
		
		OpticPack* pack = static_cast<OpticPack*>(lua_touserdata(state, -1));
		OpticRender* render = static_cast<OpticRender*>(lua_touserdata(state, -2));
		lua_pop(state, 2);

		if(args == 1) {
			render->queueAudio(pack->fetchAudio(lua_tostring(state, 1)));
		} else {
			render->queueAudio(pack->fetchAudio(lua_tostring(state, 1)), lua_toboolean(state, 2));
		}
	} catch(OpticException& e) {
		luaL_error(state, e.what());
	}

	return 0;
}

int mute_announcer(lua_State* state) {
	int args = lua_gettop(state);

	if(args != 0) {
		luaL_error(state, "mute_announcer does not take any arguments!");
	}

	try {
		Optic::Hooks::install();
	} catch(HookException& e) {
		luaL_error(state, e.what());
	}

	return 0;
}

//Temporary mess
int player_xyz(lua_State* state) {
	EngineTypes::PlayerHeader* header = *pPlayerHeader;
	int index = lua_tointeger(state, 1);
	EngineTypes::ObjectEntry* objects = (**pObjectsHeader).firstObject;
	EngineTypes::Player* players = (**pPlayerHeader).firstPlayer;
	EngineTypes::PlayerObject* player = reinterpret_cast<EngineTypes::PlayerObject*>(objects[players[index].objectIndex].offset);
	lua_pushnumber(state, player->XCoord0);
	lua_pushnumber(state, player->YCoord0);
	lua_pushnumber(state, player->ZCoord0);
	return 3;
}

void registerFunctions(lua_State* state) {
	lua_register(state, "hud_message", hud_message);
	lua_register(state, "console_print", console_print);
	lua_register(state, "distance", distance);
	lua_register(state, "current_map", current_map);
	lua_register(state, "resolution", resolution);
	lua_register(state, "register_callback", register_callback);
	lua_register(state, "field_of_view", field_of_view);
	lua_register(state, "draw_distance", draw_distance);
	lua_register(state, "game_version", game_version);
	lua_register(state, "server_name", server_name);
	lua_register(state, "server_address", server_address);
	lua_register(state, "product_type", product_type);
	lua_register(state, "in_game", in_game);
	lua_register(state, "server_max_players", server_max_players);
	lua_register(state, "server_current_players", server_current_players);
	lua_register(state, "display_medal", display_medal);
	lua_register(state, "attach_sprite", attach_sprite);
	lua_register(state, "create_sprite", create_sprite);
	lua_register(state, "create_medal", create_medal);
	lua_register(state, "create_animation", create_animation);
	lua_register(state, "sprite_properties", sprite_properties);
	lua_register(state, "add_keyframe", add_keyframe);
	lua_register(state, "medal_properties", medal_properties);
	lua_register(state, "medal_slide_animation", medal_slide_animation);
	lua_register(state, "queue_audio", queue_audio);
	lua_register(state, "player_xyz", player_xyz);
	lua_register(state, "mute_announcer", mute_announcer);
}

}};