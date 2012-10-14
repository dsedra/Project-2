#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "linkedList.h"
#include "packet.h"

/*	
	Version (1), TTL (1), Type (2)
	Sender nodeID (4)
	Sequence Number (4)
	Num Link Entries (4)
	Num Object Entries (4)
	Link Entries (variable)
	Object Entries (variable)
*/
char* createPacket(int numNeighbors, int numFiles){
	char* buf;
	// at most each objectName is 9 characters
	buf = malloc(20*sizeof(char)+numNeighbors*sizeof(int)+numFiles*9);
	return buf;
}

char* addInt(int val, char* buf){
	memcpy(buf, &val, sizeof(val));
	return buf+sizeof(val);
}
char* addChar(char val, char* buf){
	memcpy(buf, &val, sizeof(val));
	return buf+sizeof(val);
}
char* addShort(short val, char* buf){
	memcpy(buf, &val, sizeof(val));
	return buf + sizeof(val);
}
char* addString(char* str, char* buf){
	memcpy(buf, str, 9);
	return buf+9;
}

char* readInt(int* val, char* buf){
	*val = *(int *) buf;
	return buf + sizeof(int);
}
char* readChar(char* c, char* buf){
	*c = *buf;
	return buf + sizeof(char);
}
char* readShort(short* val, char* buf){
	*val = *(short *)buf;
	return buf + sizeof(short);
}
char* readString(char str[9], char* buf){
	strcpy(str, buf);
	return buf + 9;
}

// return the offset
char* addFiles(char* start){
	
	node* curr = fileList.head;
	char* ptr;
	ptr = start;
	while( curr != NULL ){
		fileEntry* fe = (fileEntry*)curr->data;
		addString(fe->objectName, ptr);
		ptr += 9;
		curr = curr->next;
	}
	return ptr;
}

char* addNodeIds(char* start, int myId){
	node* curr = routing.head;
	char* ptr;
	ptr = start;
	
	while( curr != NULL ){
		routingEntry* re = (routingEntry*)curr->data;
		if( re->nodeId != myId){
		addInt(re->nodeId, ptr);
		ptr += sizeof(int);
		}
		curr = curr->next;
	}
	return ptr;
}

char* addHeader(char* start, short type, int senderId, int seqNum, int numLinks, int numFiles){
	char* ptr = start;
	ptr = addChar('1', ptr);
	int ttl = 32;
	ptr = addChar((char)ttl,ptr);
	ptr = addShort(type, ptr);
	ptr = addInt(senderId, ptr);
	ptr = addInt(seqNum, ptr);
	ptr = addInt(numLinks, ptr);
	ptr = addInt(numFiles, ptr);
	return ptr;
}

char* readHeader(char* start, char* ttl, short* type, int* senderId, int* seqNum, int* numLinks, int* numFiles){
	char* ptr = start;
	char version;
	ptr = readChar(&version, ptr);
	ptr = readChar(ttl, ptr);
	ptr = readShort(type, ptr);
	ptr = readInt(senderId, ptr);
	ptr = readInt(seqNum, ptr);
	ptr = readInt(numLinks, ptr);
	ptr = readInt(numFiles, ptr);
	return ptr;
}

void printBuf(char* buf, int len){
	
	int i;
	for( i = 0 ; i < len ; i++){
		printf("%c", buf[i]);
	}
	printf("\nend\n");
}

void freeBuffer(char* buf){
	if(buf != NULL){
		free(buf);
	}
}

void decreaseCountDown(int udp){
	
	node* curr = reSendList.head;
	
	while( curr != NULL ){
		deferredMessage* df =(deferredMessage* ) curr->data;
		df->reTranCountDown -= advCycle;
		
		if( df->reTranCountDown == 0){
			// here resend
		} 
		
		curr = curr->next;
	}
}

void reTransmit(int udp, int receiverId, int senderId ){
	routingEntry* re = getRoutingEntry( &routing , senderId );
	char* ptr;
	if( re != NULL ){
		char* sendBuf = createPacket(re->numLinks , re->numFiles);
		ptr = addHeader(sendBuf, 1, senderId, re->seqNumReceive, re->numLinks, re->numFiles);
		
		node* curr = re->neighbors->head;
		while( curr != NULL ){
			int* id = (int* )curr->data;
		 	addInt(*id, ptr);
			ptr += sizeof(int);
			curr = curr->next;
		}
		curr = re->objects->head;
		while( curr != NULL ){
			char* objectName = (char* )curr->data;
			addString(objectName, ptr);
			ptr+=9;
			curr = curr->next;
		}
		routingEntry* neighborToReceive = getRoutingEntry( &routing, receiverId );
		if( neighborToReceive!= NULL ){
			struct sockaddr_in cli_addr;
			struct hostent *h;
			if((h = gethostbyname(re->hostName))==NULL) {
				printf("error resolving host\n");
				return;
			}
			memset(&cli_addr, '\0', sizeof(cli_addr));
			cli_addr.sin_family = AF_INET;
			cli_addr.sin_addr.s_addr = *(in_addr_t *)h->h_addr;
			cli_addr.sin_port = htons(neighborToReceive->routingPort);
			ssize_t sen;
			int packetSize = ptr - sendBuf + 1;
			
			sen = sendto(udp, sendBuf ,packetSize, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
			if(sen <= 0 ){
				printf("This is really bad\n");
			}
		}
		freeBuffer(sendBuf);
	}
}




void forward(int udp, char* packet, int packetSize, int excludeId){
	
	// here drease the TTL by one
	char* ttl = packet+1;
	*ttl = *(ttl)-1;
	
	node* curr = routing.head;
	while( curr != NULL ){
		routingEntry* re = (routingEntry*)curr->data;
		if (re->isNeighbor && re->nodeId != excludeId){
			struct sockaddr_in cli_addr;
			struct hostent *h;
			
			if((h = gethostbyname(re->hostName))==NULL) {
				printf("error resolving host\n");
				return;
			}
			memset(&cli_addr, '\0', sizeof(cli_addr));
			cli_addr.sin_family = AF_INET;
			cli_addr.sin_addr.s_addr = *(in_addr_t *)h->h_addr;
			cli_addr.sin_port = htons(re->routingPort);
			
			ssize_t sen;
			printf("Start to forward to node %d\n", re->nodeId);
			sen = sendto(udp, packet ,packetSize, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
			if(sen <= 0 ){
				printf("This is really bad\n");
			}
			
			// add each advertisement into a buffer, assumed that they are not acked yet
			// when receive ack, remove them from the buffer
			deferredMessage* msg = malloc(sizeof(deferredMessage));
			msg->receiverId = re->nodeId;
			msg->senderId = excludeId;
			msg->reTranCountDown = retranCycle;
		
			insert(&reSendList, msg, sizeof(deferredMessage));
		}
		curr = curr->next;
	}
}


void advertise(int udp , int senderId, int numLinks, int numFiles){
	node* curr = routing.head;
	char* ptr;
	
	while( curr != NULL ){
		routingEntry* re = (routingEntry*)curr->data;
		if (re->isNeighbor ){
			
			
			struct sockaddr_in cli_addr;
			struct hostent *h;
			
			if((h = gethostbyname(re->hostName))==NULL) {
				printf("error resolving host\n");
				return;
			}
			memset(&cli_addr, '\0', sizeof(cli_addr));
			cli_addr.sin_family = AF_INET;
			cli_addr.sin_addr.s_addr = *(in_addr_t *)h->h_addr;
			cli_addr.sin_port = htons(re->routingPort);
			
			char* sendBuf = createPacket(numLinks, numFiles);
			ptr = addHeader(sendBuf, 1, senderId, seqNumSend, numLinks, numFiles);
			printBuf(sendBuf, ptr-sendBuf+1);
			//printf("Header length is %ld\n", ptr-sendBuf+1);
			ptr = addNodeIds(ptr, senderId);
			printBuf(sendBuf, ptr-sendBuf+1);
			//printf("header + Nodes length is %ld\n", ptr-sendBuf+1);
			ptr = addFiles(ptr);
			int packetSize = ptr-sendBuf+1;
			printf("The packet length is %d\n",packetSize );
			
			printBuf(sendBuf, packetSize);
			ssize_t sen;
			sen = sendto(udp, sendBuf,packetSize, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
			//printf("Send to node %d on port %d\n", re->nodeId, re->routingPort);
			if(sen <= 0 ){
				printf("This is really bad\n");
			}
			
			// add each advertisement into a buffer, assumed that they are not acked yet
			// when receive ack, remove them from the buffer
			deferredMessage* msg = malloc(sizeof(deferredMessage));
			msg->receiverId = re->nodeId;
			msg->senderId = senderId;
			msg->reTranCountDown = retranCycle;
			insert(&reSendList, msg, sizeof(deferredMessage));
			
			freeBuffer(sendBuf);
		}
		curr = curr->next;
	}
	

}

void printLinkEnt(char* buf, int numLinks){
	int i;
	int* intArr = (int *)buf;
	
	for(i = 0; i < numLinks; i++){
		printf("link entry %d is %d",i,intArr[i]);
	}
}

void printFileEnt(char* buf, int numFiles){
	int i;
	
	for(i = 0; i < numFiles; i++){
		printf("file entry %d is %s",i,buf + 9*i);
	}
}













