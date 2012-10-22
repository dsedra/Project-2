#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

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
	linkedList* objects;
	
	struct re* parent;
	int visited;
	// below are normal information
	// for this node
	char* hostName;
	int routingPort;
	int localPort;
	int serverPort;
	int isNeighbor;
	//int seqNumSend;
	//int seqNumAck;
	int seqNumReceive;
	int ttl;
	int numFiles;
	int numLinks;
	
	struct sockaddr_in cli_addr;
	int neighborCountDown;
	int LSACountDown;
	int isDown;

}routingEntry;

typedef struct client{
	int socket;
	char* buffer;
}client;

extern linkedList fileList;
extern linkedList routing;
extern linkedList reSendList;
extern int numDeferMessages;
extern int mynodeID;
extern int advCycle; 


void insert( linkedList* list, void* data, int size);
void deleteRoutingEntry(int nodeId);
fileEntry* getFileEntry(linkedList* list, char* obj);
routingEntry* getRoutingEntry( linkedList* list, int nodeId );
routingEntry* pop( linkedList* list );
routingEntry* initRE(int nodeId,int visited,char* hostName,int routingPort,int localPort,int serverPort, int isNeighbor);
void printRouting(linkedList list);
void printFile(linkedList list);
fileEntry* initFL(char* objectName, char* path);
void freeList(linkedList* list);
node* freeNode(node* n);
void printRoutingEntry(routingEntry* re);
void decreaseTTL();
int resolvNeighbor(struct sockaddr_in cli_addr);
int close_socket(int sock);
int containObject(linkedList* list, char* obj);
routingEntry* getFileFromOther(linkedList* list, char* obj);
#endif
