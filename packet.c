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
		if( re->nodeId != myId && re->isNeighbor && !re->isDown){
		addInt(re->nodeId, ptr);
		ptr += sizeof(int);
		}
		curr = curr->next;
	}
	return ptr;
}

char* addHeader(char* start, int ttl, short type, int senderId, int seqNum, int numLinks, int numFiles){
	char* ptr = start;
	ptr = addChar('1', ptr);
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

deferredMessage* isExist( int senderId, int recieverId ){
	node* curr = reSendList.head;
	deferredMessage* result;
	result = NULL;
	while( curr != NULL ){
		deferredMessage* df = (deferredMessage* ) curr->data;
		if( df->inUse == 1 &&  df->receiverId == recieverId && df->senderId == senderId){
			return df;
		}
		if( df->inUse == 0){
			result = df;
		}
		curr = curr->next;
	}
	
	// this is non-exist sender, and now there is now empty spot
	if( result == NULL ){
		result = malloc(sizeof(deferredMessage));
	}
	result->inUse = 0;
	insert(&reSendList, result, sizeof(deferredMessage));
	return result;
}

void addToResendList(int senderId, int receiverId){
	deferredMessage* df = isExist( senderId, receiverId); 
	if( df ){
		// it means we find empty spot for this nonExist receiver
		if(df->inUse != 1){
		printf("Add neighbor %d to resend list\n", receiverId);
		df->inUse = 1;
		df->senderId = senderId;
		df->receiverId = receiverId;
		numDeferMessages ++ ;
		}
	}
}

void removeFromResendList( int senderId, int receiverId ){
	deferredMessage* df = isExist( senderId , receiverId);
	if( df ){
		printf("Remove neighbor %d from resend list\n", receiverId);
		df->inUse = 0;
		numDeferMessages -- ;
	}
}

void reTransmit(int udp, int receiverId, int senderId ){
	routingEntry* re = getRoutingEntry( &routing , senderId );
	char* ptr;
	if( re != NULL ){
		char* sendBuf = createPacket(re->numLinks , re->numFiles);
		ptr = addHeader(sendBuf, 32, 0, senderId, re->seqNumReceive, re->numLinks, re->numFiles);
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
		    ssize_t sen;
 		    int packetSize = ptr - sendBuf + 1;
 		   				
 		   	sen = sendto(udp, sendBuf ,packetSize, 0, (struct sockaddr *)&(neighborToReceive->cli_addr), sizeof(neighborToReceive->cli_addr));
 		   	if(sen <= 0 ){
 		   		printf("This is really bad\n");
 		   	}
		}
		freeBuffer(sendBuf);
	}
}

void doReTransmit(int udp){
	node* curr = reSendList.head;
	while( curr != NULL ){
		deferredMessage* df = (deferredMessage* ) curr->data;
		if( df->inUse == 1 ){
			printf("Retransmit to node %d\n", df->receiverId);
			reTransmit(udp, df->receiverId, df->senderId);
		}
		curr = curr->next;
	}
}

void forward(int udp, char* packet, int packetSize, int senderId, int excludeId){
	// here drease the TTL by one
	char* ttl = packet+1;
	*ttl = *(ttl)-1;
	
	node* curr = routing.head;
	while( curr != NULL ){
		routingEntry* re = (routingEntry*)curr->data;
		if (re->isNeighbor && re->nodeId != excludeId && !re->isDown){
		
			ssize_t sen;
			printf("Start to forward to node %d\n", re->nodeId);
			sen = sendto(udp, packet ,packetSize, 0, (struct sockaddr *)&(re->cli_addr), sizeof(re->cli_addr));
			if(sen <= 0 ){
				printf("This is really bad\n");
			}
			
			// add each advertisement into a buffer, assumed that they are not acked yet
			// when receive ack, remove them from the buffer
			addToResendList(senderId, re->nodeId);
		}
		curr = curr->next;
	}
}

void sendAck(int udp, int senderId, int seqNum, struct sockaddr_in cli_addr){
	char* ack = malloc(3*sizeof(int));
	char* ptr = ack;
	ptr = addChar('1', ptr);
	ptr = addChar(32,ptr);
	ptr = addShort(1, ptr);
	ptr = addInt(senderId, ptr);
	ptr = addInt(seqNum, ptr);
	
	int ackSize = ptr - ack + 1;
	ssize_t sen;
	sen = sendto(udp, ack, ackSize, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
	if(sen <= 0 ){
		printf("This is really bad\n");
	}
}


void advertise(int udp, int numLinks, int numFiles){
	node* curr = routing.head;
	char* ptr;
	routingEntry* me = getRoutingEntry(&routing, mynodeID);
	me->seqNumSend++;
	
	while( curr != NULL ){
		routingEntry* re = (routingEntry*)curr->data;
		if (re->isNeighbor){
			char* sendBuf = createPacket(numLinks, numFiles);
			ptr = addHeader(sendBuf, 32, 0, mynodeID, me->seqNumSend, me->numLinks, me->numFiles);
			//printf("Header length is %ld\n", ptr-sendBuf+1);
			ptr = addNodeIds(ptr, mynodeID);
			//printf("header + Nodes length is %ld\n", ptr-sendBuf+1);
			ptr = addFiles(ptr);
			int packetSize = ptr-sendBuf+1;
			printf("The packet length is %d\n",packetSize );
			ssize_t sen;
			printf("Advertise to node %d\n", re->nodeId);
			sen = sendto(udp, sendBuf,packetSize, 0, (struct sockaddr *)&(re->cli_addr), sizeof(re->cli_addr));
			//printf("Send to node %d on port %d\n", re->nodeId, re->routingPort);
			if(sen <= 0 ){
				printf("This is really bad\n");
			}
			
			// add each advertisement into a buffer, assumed that they are not acked yet
			// when receive ack, remove them from the buffer
			addToResendList(mynodeID, re->nodeId);
			freeBuffer(sendBuf);
		}
		curr = curr->next;
	}
}

void countDown(int udp){
	node* curr = routing.head;
	while( curr != NULL ){
		routingEntry* re = (routingEntry* )curr->data;
		if( re->isNeighbor && !re->isDown){
			re->neighborCountDown -= advCycle;
			printf("Time limit for neighbor %d : %d\n", re->nodeId, re->neighborCountDown);
			if( re->neighborCountDown == 0 ){
			char* sendBuf = createPacket(re->numLinks, re->numFiles);
			char* ptr = sendBuf;
			ptr = addHeader(sendBuf,1,0, re->nodeId, re->seqNumReceive, re->numLinks, re->numFiles);
			
			node* curr2 = re->neighbors->head;
			while( curr2 != NULL ){
				int* id = (int* )curr2->data;
			 	addInt(*id, ptr);
				ptr += sizeof(int);
				curr2 = curr2->next;
			}
			curr2 = re->objects->head;
			while( curr2 != NULL ){
				char* objectName = (char* )curr2->data;
				addString(objectName, ptr);
				ptr+=9;
				curr2 = curr2->next;
			}
			
			int packetSize = ptr - sendBuf + 1;
			forward(udp, sendBuf, packetSize, re->nodeId, re->nodeId);
			re->isDown = 1;
			//need to dcrease numlinks in us node
			routingEntry* me = getRoutingEntry(&routing, mynodeID);
			me->numLinks--;
			printf("Neighbor %d is down\n", re->nodeId);
			}	
		}
		else if( (!re->isNeighbor) && (re->nodeId != mynodeID)){
			re->LSACountDown -= advCycle;
			printf("Time limit for LSA from node %d : %d\n", re->nodeId, re->LSACountDown);
			if ( re->LSACountDown == 0){
				deleteRoutingEntry( re->nodeId );
			}
			
			
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