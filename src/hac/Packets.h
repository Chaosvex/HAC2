#ifndef PACKET_H
#define PACKET_H

#define MAX_BUFFER 1024
#define PACKET_HEADER_SIZE 4

#pragma pack(push, 1)

struct Packet {
    Packet(int id):received(0), size(0), opcode(0), uid(id) {}
    int received;
    short size;
    short opcode;
    int uid;
};

struct RawPacket : Packet {
    RawPacket(int id) : Packet(id) { };
    char buffer[MAX_BUFFER];
};

struct LoginPacket : Packet {
	LoginPacket(int id) : Packet(id) { };
    char header[4];
    wchar_t username[12];
    char password[32];
};

struct LoginPacket0 {
    short size;
	short opcode;
    char username[12];
    char password[33];
};

#pragma pack(pop)

#endif