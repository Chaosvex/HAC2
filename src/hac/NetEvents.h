#pragma once

#include <cstdint>
#include <cstddef>

namespace NetEvents {

extern std::uint32_t BackToNetHandler;

void netEventsStub();
void sendPacket(void* data, std::uint32_t length);
std::uint32_t buildChatPacket(wchar_t* chatPacket);
void sendHandshake();

} // NetEvents