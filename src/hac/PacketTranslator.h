#pragma once

#include "events.h"
#include "EventSubscriber.h"
#include <sstream>
#include <vector>

struct Packet;

class PacketTranslator : public EventSubscriber {
	DWORD run();
	void processQueue();
	bool exit;
	void doLogin(std::shared_ptr<Event> event);
	void sendPacket(Packet* packet);
	EventSubscriber* dispatcher;

public:
	void registerDispatcher(EventSubscriber* dispatcher);
};