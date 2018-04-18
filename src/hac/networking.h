#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <map>
#include "EventSubscriber.h"

struct Packet;
struct RawPacket;

enum NET_RET_CODES {
	NET_SUCCESS = 0,
	NET_WINSOCK_FAIL = 1,
	NET_RESOLVE_FAIL = 2,
	NET_CONNECT_FAIL = 3
};

class Networking : public EventSubscriber {
	friend class PacketTranslator;

public:
	Networking() : exit(false) { };
	~Networking();
	DWORD init();
	bool sendBuff(char *buff, int len);
	static DWORD WINAPI startThread(void* param);
	void processQueue();
	void registerForEvents(std::vector<EventSubscriber*>& modules);

private:
	HANDLE sConnected;
	static SOCKET sock;
	bool exit;
	bool processPacket(RawPacket* packet, int sockfd);
	fd_set master, readfds;
	std::map<int, RawPacket*> packets;
	void recvData(int sockfd);
	int highestfd;
	DWORD run();
	DWORD manage();
	bool abort;
	void dispatch(Packet* packet);
};