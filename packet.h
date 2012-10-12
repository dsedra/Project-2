#ifndef PACKET_H
#define PACKET_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* createPacket( int numNeibors, int numFiles);
char* addInt(int val, char* buf);
char* addChar(char val, char* buf);
char* addString(char* str, char* buf);
char* addFiles(char* start);
char* addNodeIds(char* start, int myId);
void advertise(int udp , int senderId, int numLinks, int numFiles);

#endif