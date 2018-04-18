#pragma once

#include "EngineTypes.h"
#include <string>
#include <cstdint>

void initCritSection();
void destroyCritSection();
void HUDMessage(const WCHAR* message);
void HUDMessage(const std::string& message);
void HUDMessage(const std::wstring& message);
void PlayMPSound(EngineTypes::multiplayer_sound index);
void ChatMessage(const WCHAR* message, EngineTypes::CHAT_TYPE type);
void GenerateMD5(const char* data, DWORD data_length, char* output);
void serverConnect(const char *address, const wchar_t *password);
void commandConnect(const std::string& ip, const std::uint16_t& port, const std::wstring& password);
void DrawDistance(float distance);
void ExecuteCommand(const char* command, bool rewind = true);
void ShowMainMenu();
void ToggleMTV(bool state);
void ShowMainMenuHaloThread();
void attentionBox(const std::string& message, bool quit = false);
void chatLocal(const std::string& message);

extern HANDLE serverJoinCritSection; //move me