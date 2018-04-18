#include "EngineTypes.h"

using namespace EngineTypes;

AnchorOffset::operator EngineTypes::Real2D() const {
	Real2D output;
	output.x = (this->x > 0.0f) ? (this->x + 0.5f) : (this->x - 0.5f);
	output.y = (this->y > 0.0f) ? (this->y + 0.5f) : (this->y - 0.5f);
	return output;
}

Real2D::operator EngineTypes::AnchorOffset() const {
	AnchorOffset output;
	output.x = static_cast<std::int16_t>(this->x);
	output.y = static_cast<std::int16_t>(this->y);
	return output;
}