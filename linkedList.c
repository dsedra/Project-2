#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
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
	//newp->seqNumSend = 0;
	//newp->seqNumAck = 0;
	newp->seqNumReceive = 0;
	newp->numFiles = 0;
	newp->numLinks = 0;
	newp->isDown = 0;
	
	newp->neighbors = malloc(sizeof(linkedList));
	newp->objects = malloc(sizeof(linkedList));
	newp->neighbors->head = NULL;
	newp->neighbors->tail = NULL;
	newp->objects->head = NULL;
	newp->objects->tail = NULL;
	
	

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
	//n->data = data;
	n->next = NULL;
	
	if ( list->head == NULL ){
		list->head = n;
		list->tail = n;
	}else{
		list->tail->next = n;
		list->tail = n;
	}
}

void deleteRoutingEntry(int nodeId){
	
	node* head = routing.head;
	
	routingEntry* re = (routingEntry*) head->data;
	if( re->nodeId == nodeId ){
		if( routing.head == routing.tail ){
			free(head);
			routing.head = NULL;
			routing.tail = NULL;
		}else{
			routing.head = routing.head->next;
			head->next = NULL;
			free(head);
		}
		return;
	}
	
	node* curr = routing.head->next;
	node* pre = routing.head;
	while( curr != NULL){
		re = (routingEntry*) curr->data;
		if( re->nodeId == nodeId ){
			pre->next = curr->next;
			if( curr->next == NULL){
				routing.tail = pre;
			}
			curr->next = NULL;
			
			freeNode(curr);
			return;
		}
		pre = curr;
		curr = curr->next;
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

node* freeNode(node* n){
	node* next;
	next = NULL;
	
	if ( n != NULL ){
		
		next = n->next;
		
		if( n->data != NULL ){
			free(n->data);
		}
		free(n);
	}
	return next;
}

void freeList(linkedList* list){
	node* curr = list->head;
	while( curr != NULL ){
		curr = freeNode( curr );
	}
	list->head = NULL;
	list->tail = NULL;
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

void printObjects(linkedList* list){
	node* currentp = list->head;
	printf("Objects: ");
    while(currentp != NULL){
        char* objectName = (char *)currentp->data;
        printf("%s  ",objectName);
        currentp = currentp->next;
    }
	printf("\n");
}

void printLinks(linkedList* list){
	node* currentp = list->head;
	printf("Neighbors: ");
    while(currentp != NULL){
        int* id = (int *)currentp->data;
        printf("%d  ",*id);
        currentp = currentp->next;
    }
	printf("\n");
}

void printRouting(linkedList list){
    node* currentp = list.head;
	printf("\n--------------ROUTING TABLE-------------\n");
    while( currentp != NULL ){
        routingEntry* entryp = (routingEntry *)currentp->data;
		if( !entryp->isDown){
		if( entryp->nodeId == mynodeID){
			printf("*Node %d\n", entryp->nodeId);
		}else{
			printf("Node %d\n", entryp->nodeId);
		}
		printObjects(entryp->objects);
		printLinks(entryp->neighbors);
	}
        currentp = currentp->next;
    }
	printf("----------------------------------------\n\n");
}

void printRoutingEntry(routingEntry* re){
	
	printf("NodeId: %d\n", re->nodeId);
	printf("TTL: %d\n", re->ttl);
	printf("Seq Num received: %d\n", re->seqNumReceive);
	printf("Number of files: %d\n", re->numFiles);
	printf("Number of links: %d\n", re->numLinks);
}

int resolvNeighbor(struct sockaddr_in cli_addr){
	//printf("The host is: %s\n",inet_ntoa(cli_addr.sin_addr));
	//printf("The port is: %d\n", ntohs(cli_addr.sin_port));
	node* curr = routing.head;
	while( curr != NULL){
		routingEntry* re = (routingEntry *)curr->data;
		if( re->isNeighbor){
			if( strcmp(re->hostName, inet_ntoa(cli_addr.sin_addr)) == 0 && re->routingPort ==ntohs(cli_addr.sin_port) ){
				return re->nodeId;
			}
		}
		curr = curr->next;
	}
	
	return -1;
}


void decreaseTTL(){
	node* curr = routing.head;
	while ( curr != NULL ){
		routingEntry* entryp = (routingEntry *)curr->data;
		entryp->ttl -= 1;
		curr = curr->next;
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



