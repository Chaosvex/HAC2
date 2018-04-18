#pragma once

#include <queue>
#include <Windows.h>
#include <iostream>
//#include "logger.h"
#include "packet.h"
#include "opcodes.h"
#include <map>

typedef void (*handlerFunc)(Packet*);

using namespace std;

class Dispatcher {
public:
	Dispatcher();
	~Dispatcher();
	bool run();
	void push(Packet* packet);
	//static pthread_cond_t queueProcess;
	//static pthread_mutex_t processMutex;
	int size();
	volatile bool bRun;
	//static pthread_mutex_t queueLock;
	bool registerHandler(short opcode, handlerFunc handler);
	void unregisterHandler(short opcode);

private:
	static queue<Packet*> packets;
	static map<short, handlerFunc> dispatchTable;
	void pop();
	Packet* pop_front();
	void process();
	static void defaultHandler(Packet* packet);
};