#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "linkedList.h"

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
char* addString(char* str, char* buf){
	memcpy(buf, str, 9);
	return buf+9;
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

char* addHeader(char* start, char type, int senderId, int seqNum, int numLinks, int numFiles){
	char* ptr = start;
	ptr = addChar('1', ptr);
	// there is only one byte for this filed, really?
	ptr = addChar('9',ptr);
	ptr = addChar(type, ptr);
	ptr = addInt(senderId, ptr);
	ptr = addInt(seqNum, ptr);
	ptr = addInt(numLinks, ptr);
	ptr = addInt(numFiles, ptr);
	return ptr;
}

void printBuf(char* buf, int len){
	
	int i;
	for( i = 0 ; i < len ; i++){
		printf("%c", buf[i]);
	}
	printf("\nend\n");
}


void advertise(int udp , int senderId, int numLinks, int numFiles){
	node* curr = routing.head;
	char* ptr;
	
	while( curr != NULL ){
		routingEntry* re = (routingEntry*)curr->data;
		if (re->isNeighbor){
			
			
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
			ptr = addHeader(sendBuf, '1', senderId, re->seqNumSend, numLinks, numFiles);
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
			
			
			
		}
		curr = curr->next;
	}
}





