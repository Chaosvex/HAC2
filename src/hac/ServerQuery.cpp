#include "ServerQuery.h"
#include "ServerInformation.h"
#include <WinSock2.h>

//@todo - move WinSock open/close to constructor/destructor
ServerQuery::ServerQuery(const char* ip, unsigned short port, long tSeconds, long tMicros) : ip(ip) {
	this->port = port;
	this->tSecs = tSeconds;
	this->tMicros = tMicros;
}

ServerQuery::ServerQuery(const ServerInfo& info, long tSeconds, long tMicros) : ip(info.ip) {
	this->port = info.port;
	this->tSecs = tSeconds;
	this->tMicros = tMicros;
}

bool ServerQuery::query(char* buffer, int buffLen) {
	bool success = false;
    WSADATA wsaData;

    SOCKET SendSocket = INVALID_SOCKET;
    sockaddr_in RecvAddr;
	timeval timeout = {tSecs, tMicros};

    char *SendBuf = "\\query";
    int BufLen = 6;
	fd_set sockets;

	FD_ZERO(&sockets);

    //Init Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    
	if (iResult != NO_ERROR) {
        OutputDebugString("WSAStartup failed");
        return false;
    }

    // Create socket
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
	if (SendSocket == INVALID_SOCKET) {
        OutputDebugString("Unable to create socket");
        WSACleanup();
        return false;
    }

	//Add to master set
	FD_SET(SendSocket, &sockets);

    //Set up the RecvAddr structure with the IP address of
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(this->port);
	RecvAddr.sin_addr.s_addr = inet_addr(this->ip.c_str());

	//Send query packet
    iResult = sendto(SendSocket, SendBuf, BufLen, 0, (SOCKADDR *) &RecvAddr, sizeof(RecvAddr));
    
	if (iResult == SOCKET_ERROR) {
        closesocket(SendSocket);
        WSACleanup();
        return false;
    }
	
	if(select(SendSocket + 1, &sockets, NULL, NULL, &timeout) == -1) {
		return false;
	} else {
		SOCKADDR from;
		int len = sizeof(SOCKADDR);
		memset(buffer, 0, buffLen);

		//Time to receive the query response
		if(FD_ISSET(SendSocket, &sockets)) {
			iResult = recvfrom(SendSocket, buffer, buffLen, 0, (SOCKADDR *) &from, &len);
			success = iResult != SOCKET_ERROR;
		} else {
			success = false;
		}
	}

    WSACleanup();
	return success;
}
