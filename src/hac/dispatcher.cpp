#include "dispatcher.h"

queue<Packet*> Dispatcher::packets;
map<short, handlerFunc> Dispatcher::dispatchTable;
/*pthread_mutex_t Dispatcher::queueLock;
pthread_mutex_t Dispatcher::processMutex;
pthread_cond_t Dispatcher::queueProcess;

Dispatcher::Dispatcher():bRun(true) {
	pthread_mutex_init(&queueLock, NULL);
	pthread_mutex_init(&processMutex, NULL);
	pthread_cond_init(&queueProcess, NULL);

	//Register default handler
	for(short i = LOWEST_OPCODE; i <= HIGHEST_OPCODE; i++) {
		registerHandler(i, defaultHandler);
	}
}

/*
* Destroy mutexes and free unhandled packets
*//*
Dispatcher::~Dispatcher() {
	pthread_mutex_destroy(&queueLock);
	pthread_mutex_destroy(&processMutex);
	pthread_cond_destroy(&queueProcess);

	//Clean up unserviced packets
	for(unsigned int i = 0; i < packets.size(); i++) {
		delete packets.front();
		packets.pop();
	}
}

bool Dispatcher::run() {
	while(bRun) {
		pthread_mutex_lock(&processMutex);
		pthread_cond_wait(&queueProcess, &processMutex);
		pthread_mutex_unlock(&processMutex);
		process();
	}

	return false;
}*/

void Dispatcher::process() {
	while(size() > 0) {
		Packet* packet = pop_front();
		if(packet->opcode >= LOWEST_OPCODE && packet->opcode <= HIGHEST_OPCODE) {
			(*dispatchTable[packet->opcode])(packet);
		} else {
			delete packet;
		}
	}
}

bool Dispatcher::registerHandler(short opcode, handlerFunc handler) {
	dispatchTable[opcode] = handler;
	return true;
}

void Dispatcher::unregisterHandler(short opcode) {
	dispatchTable.erase(opcode);
}

void Dispatcher::defaultHandler(Packet* packet) {
	delete packet;
}

/*
* All methods below this line should be removed later.
* Make a thread safe wrapper around queue instead.
*/
void Dispatcher::push(Packet* packet) {
	pthread_mutex_lock(&queueLock);
	packets.push(packet);
	pthread_mutex_unlock(&queueLock);
}

void Dispatcher::pop() {
	pthread_mutex_lock(&queueLock);
	packets.pop();
	pthread_mutex_unlock(&queueLock);
}

Packet* Dispatcher::pop_front() {
	pthread_mutex_lock(&queueLock);
	Packet* packet = packets.front();
	packets.pop();
	pthread_mutex_unlock(&queueLock);
	return packet;
}

int Dispatcher::size() {
	pthread_mutex_lock(&queueLock);
	int size = packets.size();
	pthread_mutex_unlock(&queueLock);
	return size;
}
