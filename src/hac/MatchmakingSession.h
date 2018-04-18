#pragma once

#include "EventSubscriber.h"
#include <vector>
#include <WinDef.h>
#include <map>
#include <memory>

namespace Matchmaking {

class Session : public EventSubscriber {
	DWORD run();
	void process_queue();
	bool exit_;
	EventSubscriber* dispatcher_;
	HANDLE exit_sem_;

public:
	void wait();
	Session(EventSubscriber* dispatcher);
	~Session();
	void registerDispatcher(EventSubscriber* dispatcher);
};

} // Matchmaking

