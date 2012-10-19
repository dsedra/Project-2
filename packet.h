#ifndef PACKET_H
#define PACKET_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct df{
	int inUse;
	int receiverId;
	int senderId;
}deferredMessage;


extern int advCycle;
extern int retranCycle;

char* createPacket( int numNeibors, int numFiles);
char* addInt(int val, char* buf);
char* addChar(char val, char* buf);
char* addString(char* str, char* buf);
char* addFiles(char* start);
char* addNodeIds(char* start, int myId);
void advertise(int udp, int numLinks, int numFiles);
void forward(int udp, char* packet, int packetSize, int senderId, int excludeId);
void sendAck(int udp, int senderId, int seqNum, struct sockaddr_in cli_addr);
deferredMessage* isExist( int senderId, int receiverId );
void addToResendList(int senderId, int receiverId);
void removeFromResendList( int senderId, int receiverId );
void doReTransmit(int udp);
void printLinkEnt(char* buf, int numLinks);
void printFileEnt(char* buf, int numFiles);
char* readInt(int* val, char* buf);
char* readChar(char* c, char* buf);
char* readShort(short* val, char* buf);
char* readString(char str[9], char* buf);
char* readHeader(char* start, char* ttl, short* type, int* senderId, int* seqNum, int* numLinks, int* numFiles);
void countDown(int udp);
void reTransmit(int udp, int receiverId, int senderId );
#endif