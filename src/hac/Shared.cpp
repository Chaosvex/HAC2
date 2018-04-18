#include "Shared.h"
#include "ServerInformation.h"

ServerInfo lastServer;
char* mapSearchPath;
wchar_t reconnectPassword[PASSWORD_LENGTH];
wchar_t* serverName;
char* serverAddress;
char* productType;
std::uint8_t* activeCamoDisable;
std::uint8_t* masterVolume;
float* tickrate;
float* gravity;
std::int32_t timeleft;
ServerDetails serverInfo;