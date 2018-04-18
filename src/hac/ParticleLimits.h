#pragma once

#include <cstdint>

namespace EngineLimits { namespace Particles {

void set(std::uint16_t limit);
void restore();

}};