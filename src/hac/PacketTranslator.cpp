#include "PacketTranslator.h"
#include "Packets.h"
#include "networking.h"
#include "opcodes.h"

DWORD PacketTranslator::run() {
	exit = false;

	while(!exit) {
		WaitForSingleObject(eventSem, INFINITE); //block while no events
		processQueue();
	}

	return 0;
}

void PacketTranslator::processQueue() {
	std::shared_ptr<Event> event;

	while(!events.empty()) {
		if(events.try_pop(event)) {
			switch(event->type) {
				case THREAD_EXIT:
					exit = true;
					break;
				case LOGIN_ATTEMPT:
					doLogin(event);
					break;
			}
		}
	}
}

void PacketTranslator::registerDispatcher(EventSubscriber* dispatcher) {
	dispatcher->subscribe(this, LOGIN_ATTEMPT);
	dispatcher->subscribe(this, CONNECTION_LOST);
	this->dispatcher = dispatcher;
}

void PacketTranslator::doLogin(std::shared_ptr<Event> event) {
	LoginEvent *e = (LoginEvent*)event.get();
	LoginPacket0 *packet = new LoginPacket0();
	strcpy_s(packet->username, 12, e->username.c_str());
	strcpy_s(packet->password, 33, e->password.c_str());
	packet->opcode = 10;
	packet->size = 48;
	sendPacket((Packet*)packet);
	delete packet;
}

void PacketTranslator::sendPacket(Packet* packet) {
	DWORD remaining = 48;

	while(remaining) {
		int res = send(Networking::sock, (char*)(packet + 48 - remaining), remaining, 0);

		if(res == SOCKET_ERROR) {
			return;
		}

		remaining -= res;
	}
}