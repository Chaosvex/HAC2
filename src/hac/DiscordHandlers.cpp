#include "DiscordHandlers.h"
#include "DebugHelper.h"
#include "EnginePointers.h"
#include "EngineState.h"
#include "Shared.h"
#include <cstring>
#include <ctime>
#include <cstdio>

namespace DiscordHandlers {

void ready() {
    DebugHelper::Translate("Hello");
}

void error(int errorCode, const char* message) {
    DebugHelper::Translate("Hello2");
}

void disconnected(int errorCode, const char* message) {
    DebugHelper::Translate("Hello3");
}

void join(const char* joinSecret) {
    DebugHelper::Translate("Hello4");
}

void spectate(const char* spectateSecret) {
    DebugHelper::Translate("Hello5");
}

void joinRequest(const DiscordJoinRequest* request) {
    DebugHelper::Translate("Hello6");
}

void presenceUpdate() {
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));

    char dest[255];

    if(strcmp(pCurrentMap, "ui") != 0) {
        discordPresence.state = serverAddress;
        discordPresence.largeImageText = pCurrentMap;
        discordPresence.largeImageKey = pCurrentMap;
        discordPresence.smallImageKey = "nametest";
        discordPresence.smallImageText = pCurrentMap;
        discordPresence.partyId = "444";
        discordPresence.partySize = 1;
        discordPresence.partyMax = 16;
        wcstombs(dest, serverName, sizeof(dest));
        discordPresence.details = dest;

        if(timeleft) {
            discordPresence.endTimestamp = time(nullptr) + (timeleft / 30);
        }
    } else {
        discordPresence.state = "Idling";
        discordPresence.largeImageKey = "halo";
        discordPresence.largeImageText = "Main Menu";
    }

    discordPresence.instance = 1;
    Discord_UpdatePresence(&discordPresence);
}

} // DiscordHandlers