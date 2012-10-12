#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "linkedList.h"

routingEntry* initRE(int nodeId,int visited,char* hostName,int routingPort, int localPort,int serverPort, int isNeighbor){
    
    routingEntry* newp = malloc(sizeof(routingEntry));
    newp->nodeId = nodeId;
    newp->visited = visited;
    newp->hostName = malloc(strlen(hostName) + 1);
    strcpy(newp->hostName,hostName);
    newp->routingPort = routingPort;
    newp->localPort = localPort;
    newp->serverPort = serverPort;
	newp->isNeighbor = isNeighbor;
	newp->seqNumSend = 0;
	newp->seqNumAck = 0;
	newp->seqNumRecieve = 1;
	
	newp->neighbors = malloc(sizeof(linkedList));
	newp->neighbors->head = NULL;
	newp->neighbors->tail = NULL;
	

    return newp;
}

fileEntry* initFL(char* objectName, char* path){
    
    fileEntry* newp = malloc(sizeof(fileEntry));
    newp->objectName = malloc(strlen(objectName) + 1);
    strcpy(newp->objectName, objectName);
    newp->path = malloc(strlen(path) + 1);
    strcpy(newp->path, path);
    
    return newp;
}

void insert(linkedList* list, void* data, int size){
	
	node* n;
	n = malloc( sizeof(node) );
	n->data = malloc(size);
	
	memcpy(n->data, data, size);
	n->next = NULL;
	
	if ( list->head == NULL ){
		list->head = n;
		list->tail = n;
	}else{
		list->tail->next = n;
		list->tail = n;
	}
}

fileEntry* getFileEntry(linkedList* list, char* obj){
	node* curr;
	curr = list->head;
	while( curr != NULL ){
		fileEntry* fe = (fileEntry*)curr->data;
		if ( strcmp(fe->objectName, obj) == 0){
			return fe;
		}
		curr = curr->next;
	}
	return NULL;
}

routingEntry* getRoutingEntry( linkedList* list, int nodeId ){
	node* curr;
	curr = list->head;
	while( curr != NULL ){
		routingEntry* re = (routingEntry* )curr->data;
		if( re->nodeId == nodeId){
			return re;
		}
		curr = curr->next;
	}
	return NULL;
}

routingEntry* pop( linkedList* list ){
	
	// which is an empty list
	if( list->head == NULL ){
		return NULL;
	}
	node* curr;
	curr = list->head;
	list->head = curr->next;
	
	if (list->head == NULL){
		list->tail = NULL;
	}
	curr->next = NULL;
	routingEntry* re = (routingEntry* )curr->data;
	return re;
}

void printFile(linkedList list){
	node* currentp = list.head;

    while(currentp != NULL){
        fileEntry* entryp = (fileEntry *)currentp->data;
        
        printf("%s\n",entryp->objectName);
        printf("%s\n",entryp->path);
        currentp = currentp->next;
    }
    
}
void printRouting(linkedList list){
    node* currentp = list.head;
    
    while(currentp != NULL){
        routingEntry* entryp = (routingEntry *)currentp->data;
		printf("%d\n", entryp->nodeId);
	
		node* curr;
		curr = entryp->neighbors->head;
		while( curr != NULL){
			routingEntry* nb = (routingEntry*) curr->data;
			printf("---Neighbor %d\n", nb->nodeId);
			curr = curr->next;
		}
        currentp = currentp->next;
    }
    
}
int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}



