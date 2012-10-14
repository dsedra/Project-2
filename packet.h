#ifndef PACKET_H
#define PACKET_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct df{
	int receiverId;
	int senderId;
	int reTranCountDown;
}deferredMessage;


extern int advCycle;
extern int retranCycle;

char* createPacket( int numNeibors, int numFiles);
char* addInt(int val, char* buf);
char* addChar(char val, char* buf);
char* addString(char* str, char* buf);
char* addFiles(char* start);
char* addNodeIds(char* start, int myId);
void advertise(int udp , int senderId, int numLinks, int numFiles);
void forward(int udp, char* packet, int packetSize, int excludeId);


void printLinkEnt(char* buf, int numLinks);
void printFileEnt(char* buf, int numFiles);
char* readInt(int* val, char* buf);
char* readChar(char* c, char* buf);
char* readShort(short* val, char* buf);
char* readString(char str[9], char* buf);
char* readHeader(char* start, char* ttl, short* type, int* senderId, int* seqNum, int* numLinks, int* numFiles);
#endif