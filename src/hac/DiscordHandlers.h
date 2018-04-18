#pragma once

#include <discord-rpc.h>

#define DISCORD_APP_ID "378294396438839302"

namespace DiscordHandlers {

void ready();
void error(int errorCode, const char* message);
void disconnected(int errorCode, const char* message);
void join(const char* joinSecret);
void spectate(const char* spectateSecret);
void joinRequest(const DiscordJoinRequest* request);

void presenceUpdate();

} // DiscordHandlers