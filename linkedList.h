#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node_s{
	void* data;
	struct node_s *next;
}node;

typedef struct list{
	node* head;
	node* tail;
}linkedList;

typedef struct fe{
	char* objectName;
	char* path;
}fileEntry;

typedef struct re{
	int nodeId;
	int nextHop;
	linkedList* neighbors;
	struct re* parent;
	int visited;
	// below are normal information
	// for this node
	char* hostName;
	int routingPort;
	int localPort;
	int serverPort;
	int isNeighbor;
	int seqNumSend;
	int seqNumAck;
	int seqNumRecieve;
	
}routingEntry;

typedef struct client{
	int socket;
	char* buffer;
}client;

extern linkedList fileList;
extern linkedList routing;

void insert( linkedList* list, void* data, int size);
fileEntry* getFileEntry(linkedList* list, char* obj);
routingEntry* getRoutingEntry( linkedList* list, int nodeId );
routingEntry* pop( linkedList* list );
routingEntry* initRE(int nodeId,int visited,char* hostName,int routingPort,int localPort,int serverPort, int isNeighbor);
void printRouting(linkedList list);
void printFile(linkedList list);
fileEntry* initFL(char* objectName, char* path);
int close_socket(int sock);

#endif
