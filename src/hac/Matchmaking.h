#pragma once

#include <cstdint>

namespace Matchmaking {

enum /*class*/ Mode {
	SMALL,  // 2s
	MEDIUM, // 3, 4, 5
	LARGE   // 6, 7, 8
};

void start(Mode mode);
void rejoin(std::uint32_t game_id);
void stop();

} // Matchmaking