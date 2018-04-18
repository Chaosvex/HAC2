#include "networking.h"
#include "Packets.h"

const char* SERVER_PORT = "3750";

#ifdef _DEBUG
	const char* SERVER_ADDRESS = "server2.touringnet.co.uk";
#else
	const char* SERVER_ADDRESS = "master.haloanticheat.com";
#endif

SOCKET Networking::sock;

Networking::~Networking() {	

}

bool Networking::sendBuff(char *buff, int len) {
	int sent = 0;
	
	while(sent < len) {
		int result;
		if((result = send(sock, buff, len, 0)) == SOCKET_ERROR) {
			OutputDebugString("Socket error on sending!");
			closesocket(sock);
			WSACleanup();
			return false;
		} else {
			sent += result;
		}
	}

	return true;
}

DWORD Networking::run() {
	//If the loop broke due to an error, reconnect
	while(!exit) {
		manage();
		while(!exit && WaitForSingleObject(sConnected, 2000) != WAIT_OBJECT_0) {
			processQueue();
			init();
		}
	}
	return 0;
}

DWORD Networking::manage() {
	timeval timeout = {0, 10};

	while(!exit) {
		timeout.tv_usec = 0;
		timeout.tv_sec = 2;
		readfds = master;

		if(select(highestfd+1, &readfds, NULL, NULL, &timeout) == -1) {
			//logger->writeln("Networking error", Logger::ERROR);
			break;
		}

		//Iterate over the connections
		for(int i = 0; i <= highestfd; i++) {
			if(FD_ISSET(i, &readfds)) { //if select FD is in the read set
				this->recvData(i);
			}
		}

		processQueue();

		if(abort) {
			break;
		}
	}

	processQueue();
	closesocket(sock);
	WSACleanup();
	return 0;
}

//None of this should be here
void Networking::processQueue() {
	std::shared_ptr<Event> event;

	/*while(!events.empty()) {
		if(events.try_pop(event)) {
			switch(event->type) {
				case THREAD_EXIT:
					exit = true;
					break;
				case CONNECTION_LOST:
					//PlayMPSound(_multiplayer_sound_flag_failure);
					HUDMessage(L"Lost connection to Halo Anticheat!");
					hkDrawText("Retrying connection...", C_TEXT_RED);
					break;
				case CONNECTION_ESTABLISHED:
					//PlayMPSound(_multiplayer_sound_countdown_timer_end);
					HUDMessage(L"Connection to Halo Anticheat established.");
					//hkDrawText("Connection to Halo Anticheat established.", C_TEXT_GREEN);
					break;
			}
		}
	}*/
}

void Networking::recvData(int sockfd) {
	std::string message;
	int nbytes;
	int bufferSpace = MAX_BUFFER - packets[sockfd]->received;
	char *buffer = packets[sockfd]->buffer + (MAX_BUFFER - bufferSpace);

	if((nbytes = recv(sockfd, buffer, bufferSpace, 0)) > 0) {
		packets[sockfd]->received += nbytes;
		processPacket(packets[sockfd], sockfd);
	} else {
		if(nbytes == 0) {
			message = "Connection closed";
		} else {
			message = "Receive error - ";
			message += WSAGetLastError();
		}

		//Delete associated packet and close
		delete packets[sockfd];
		packets.erase(sockfd);
		FD_CLR(sockfd, &master);
		//logger->writeln(log, Logger::MESSAGE);
		OutputDebugString(message.c_str());
		abort = true;

		//Send connection lost event
		std::shared_ptr<Event> event(std::make_shared<Event>(CONNECTION_LOST));
		broadcast(event);
	}
}

DWORD WINAPI Networking::startThread(void* param) {
	Networking *handle = static_cast<Networking*>(param);
	return handle->manage();
}

void Networking::dispatch(Packet* packet) {
}

/*
 * All of the mallocs and copies in this function make it
 * pretty inefficient. Find a better solution, one day.
 */
bool Networking::processPacket(RawPacket* packet, int sockfd) {
	//Read the header and update the packet
	if(packet->received >= PACKET_HEADER_SIZE) {
		if(packet->size == 0) {
			//Size = header + payload
			packet->size = *(short*)&packet->buffer;

			if(packet->size < PACKET_HEADER_SIZE) {
				//Delete invalid packet and pretend it never happened
				OutputDebugString("Malformed packet header");
				packets[sockfd] = new RawPacket(sockfd);
				delete packet;
				return false;
			}
		}

		//Is the packet complete?
		if(packet->received >= packet->size) {
			bool recall = false; //hacky
			packet->opcode = *(short*)&(packet->buffer[2]);
			//Start of next packet may have been merged
			int overflow = packet->received - packet->size;

			if(overflow > 0) {
				//Store the overflow in a new packet and copy the buffer
				packets[sockfd] = new RawPacket(sockfd);
				memmove(&packets[sockfd]->buffer, &packet->buffer[packet->size], overflow);
				packets[sockfd]->received = overflow;
				recall = true;
			} else {
				//Packet is complete, no overflow to process
				packets[sockfd] = new RawPacket(sockfd);
			}

			dispatch(packet);

			//Pretty hacky fix
			if(recall) {
				processPacket(packets[sockfd], sockfd);
			}
		}
	}

	return true;
}

DWORD Networking::init() {
	sConnected = CreateSemaphore(NULL, 0, 1, NULL);
	abort = false;

	//Initialise Winsock
	WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if(res != 0) {
		OutputDebugString("Winsock init failed!");
		return NET_WINSOCK_FAIL;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;
	FD_ZERO(&master);
	FD_ZERO(&readfds);
	ZeroMemory(&hints, sizeof(hints));
	highestfd = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//Resolve server address and port
	res = getaddrinfo(SERVER_ADDRESS, SERVER_PORT, &hints, &result);
	
	if(res != 0) {
		OutputDebugString("getaddrinfo failed!");
		WSACleanup();
		return NET_RESOLVE_FAIL;
	}

	//Connect to the first IP returned by getaddrinfo
	ptr = result;

	//Create a socket for connecting to the server
	sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if(sock == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		return NET_WINSOCK_FAIL;
	}

	//Connect to server
	res = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);

	if(res == SOCKET_ERROR) {
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
	
	freeaddrinfo(result);

	if(sock == INVALID_SOCKET) {
		WSACleanup();
		return NET_CONNECT_FAIL;
	}

	//Add socket descriptor to master set
	FD_SET(sock, &master);
	highestfd = sock;

	//Create a new packet for the connection
	packets[sock] = new RawPacket(sock);

	//Send connection established event
	//shared_ptr<Event> event(new Event(CONNECTION_ESTABLISHED));
	//broadcast(event);

	ReleaseSemaphore(sConnected, 1, NULL);
	return NET_SUCCESS;
}

void Networking::registerForEvents(std::vector<EventSubscriber*>& modules) {
	for(size_t i = 0; i < modules.size(); i++) {
		modules[i]->subscribe(this, THREAD_EXIT);
		//modules[i]->subscribe(this, CONNECTION_LOST);
		//modules[i]->subscribe(this, CONNECTION_ESTABLISHED);
	}
}