#include "EngineState.h"

namespace {
	int UIState = EngineState::DEFAULT;
}

bool EngineState::checkState(STATE state) {
	return (state & UIState) != 0;
}

void EngineState::toggleState(STATE state) {
	UIState ^= state;
}

void EngineState::clearState(STATE state) {
	UIState &= ~state;
}

void EngineState::setState(STATE state) {
	UIState |= state;
}