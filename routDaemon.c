#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include "linkedList.h"
#include "packet.h"
#include "algo.h"

#define mynode1Conf "own.conf"
#define mynode1Files "node1.files"

#define max(a,b) ((a>b)?a:b)

linkedList fileList;
linkedList routing;
linkedList reSendList;

int numDeferMessages;

int advCycle = 10;
int retranCycle = 3;
int neighborTimeout = 30;
int LSATimeout = 30;
int mynodeID = 50;

typedef struct timeStruct{	
	int timeout;
	int pipe;
}timeStruct;

void *timer(void *param){
	char string[30] = "Wakeup!";
	timeStruct time = *((timeStruct *) param);
	
	while(1){
		sleep(time.timeout);
		write(time.pipe, string, strlen(string));
	}
	
}
int runTimer(int timeout){
	pthread_t tid;
	int timePipe[2];
	timeStruct* tsp = malloc(sizeof(timeStruct));

	pipe(timePipe);
	tsp->timeout = timeout;
	tsp->pipe = timePipe[1];
	
	pthread_create(&tid, NULL, timer, tsp);

	return timePipe[0];
}

int main(int argc, char** argv){
	
    FILE* conf;
    FILE* files;
    char buf[256];
	char udpReadBuf[1500];
    routingEntry* neighbor;       
    fileEntry* ourFiles;
	int numNeighbors;
	int numFiles;
	numNeighbors = 0;
	numFiles = 0;
	
	
	// these are for TCP listening
	int localSocket;
	int cgiSocket;
	struct sockaddr_in cgiAddr;
	unsigned int cgiAddrLength;
	char buffer[10];
	ssize_t re;
	memset(buffer, '\0', 10);
	char method[10];
	char objectName[100];
	int objLength;
	int pathLength;
	char path[100];
	char response[256];
	fd_set sockList, master;
	int fdmax;
	char* clientArr[1024];
	int p;
	
	
	
	// these are for UDP listening
    int remoteSocket;
    struct sockaddr_in udpAddr, outAddr;
	unsigned int udpAddrLength;
	char ttlUDP;
	short typeUDP;
	int senderIdUDP;
	int seqNumberUDP;
	int numLinksUDP;
	int numFilesUDP;

	
	for(p = 0; p < 1024; p++){
		clientArr[p] = malloc(1);
		*(clientArr[p]) = '\000'; 
	}
	
	/* TODO add command line */
    
    /* Initialize file list and routing list */
    fileList.head = NULL;
    fileList.tail = NULL;
	routing.head = NULL;
    routing.tail = NULL;
    
    /* Open config and files */
    if((conf = fopen(mynode1Conf, "r")) == NULL){
        printf("Error opening the configuration file\n");
        return EXIT_FAILURE;
    }
    if((files = fopen(mynode1Files, "r")) == NULL){
        printf("Error opening the files\n");
        return EXIT_FAILURE;
    }
       
    /* initialize our */
	routingEntry* usp;

    while(fgets(buf,sizeof(buf),conf)){
     int nodeID,routingPort,localPort,serverPort; 
     char hostName[200]; 	
        if(sscanf(buf,"%d %s %d %d %d",&nodeID,hostName,&routingPort,&localPort,&serverPort) < 5){
            printf("Couldn't parse config file\n");
            break;
        }
        
		if ( nodeID == mynodeID){
			usp = initRE(mynodeID, 0,hostName,
		                           routingPort,localPort,serverPort, 0);
		}else{

        neighbor = initRE(nodeID,0,hostName,routingPort,localPort,serverPort, 1);
		struct hostent *h;
		if((h = gethostbyname(hostName))==NULL) {
			printf("error resolving host\n");
			break;
		}
		memset(&neighbor->cli_addr, '\0', sizeof(neighbor->cli_addr));
	   	neighbor->cli_addr.sin_family = AF_INET;
		neighbor->cli_addr.sin_addr.s_addr = *(in_addr_t *)h->h_addr;
  		neighbor->cli_addr.sin_port = htons(routingPort);
		neighbor->neighborCountDown = neighborTimeout;
        insert(&routing, neighbor, sizeof(routingEntry));
		}
    }
    
    while(fgets(buf,sizeof(buf),files)){
        char objectName[500];
        char path[500];
        
         if(sscanf(buf,"%s %s",objectName,path) < 2){
             printf("Couldn't parse files\n");
             break;
        }
           
        ourFiles = initFL(objectName,path);
        insert(&fileList, ourFiles, sizeof(fileEntry));
		insert(usp->objects, objectName, 9);
		usp->numFiles++;
   	}	
	
	insert(&routing, usp, sizeof(routingEntry));
	routingEntry* me = getRoutingEntry(&routing, mynodeID);
    fclose(conf);
    fclose(files);
      
	// here we add neighbors into me node
	node* trav = routing.head;
	while( trav != NULL ){
		routingEntry* re = (routingEntry* ) trav->data;
		if( re->isNeighbor ){
			//int nId = re->nodeId;
			int* nId = malloc(sizeof(int));
			*nId = re->nodeId;
			insert(me->neighbors, nId, sizeof(int));
			me->numLinks++;
		}
		trav = trav->next;
	}

	printf("hostName: %s localPort: %d routingPort: %d serverPort: %d\n\n", \
	 me->hostName, me->localPort, me->routingPort, me->serverPort);
	
    printRouting(routing);
	//printFile(fileList);
	
	// here we setup the resend message list	
	int u;
	numDeferMessages = 0;
	for(u = 0; u< me->numLinks ; u++){
		deferredMessage* df;
		df = malloc(sizeof(deferredMessage));
		df->inUse = 0;
		insert(&reSendList, df, sizeof(deferredMessage));
	}
	
	// create a TCP socket, and listen on it
	localSocket = socket(PF_INET, SOCK_STREAM, 0);
	cgiAddr.sin_family = AF_INET;
    cgiAddr.sin_port = htons(me->localPort);
    cgiAddr.sin_addr.s_addr = INADDR_ANY;
	cgiAddrLength = sizeof(cgiAddr);
	
	if( bind(localSocket, (struct sockaddr *) &cgiAddr, sizeof(cgiAddr)) < 0 ){
		close_socket(localSocket);
		fprintf(stderr, "Binding socket failed.\n");
		return EXIT_FAILURE;
	}
	if (listen(localSocket, 1) < 0 )
    {
        close_socket(localSocket);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }
	fprintf(stdout, "----- Routing Start -----\n");
	
	// create a UDP socket
    remoteSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(me->routingPort);
    udpAddr.sin_addr.s_addr = INADDR_ANY;
    udpAddrLength = sizeof(udpAddr);
    
    if(bind(remoteSocket, (struct sockaddr *) &udpAddr, udpAddrLength) < 0){
        close_socket(remoteSocket);
        fprintf(stderr, "Binding remote socket failed.\n");
        return EXIT_FAILURE;
    }
	
	
	int advTimer = runTimer(advCycle);
	int reTranTimer = runTimer( retranCycle );
	FD_SET(remoteSocket, &master);
	FD_SET(localSocket, &master);
	FD_SET(advTimer, &master);
	FD_SET(reTranTimer, &master);
	fdmax = reTranTimer;
	
	while(1){
		sockList = master;
		int i;
		//printf("before select\n");
		if(select(fdmax+1, &sockList, NULL, NULL, NULL) < 0){
			fprintf(stderr, "Select failed.\n");
			exit(1);
		}
		
		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i, &sockList)){
				if(i == localSocket){
					printf("I am here\n");
					if((cgiSocket = accept(localSocket, (struct sockaddr *) &cgiAddr, &cgiAddrLength)) == -1){
						fprintf(stderr, "Error accepting.\n");
						close_socket(localSocket);
					}
					else{
						FD_SET(cgiSocket, &master);
						fdmax = max(fdmax, cgiSocket); 
					}
				}
				// advertisement or ack comes into this routing table
				else if (i == remoteSocket){
					
					memset(&outAddr, '\0', sizeof(outAddr));
					unsigned int clilen;
					clilen = sizeof(outAddr);
					if(recvfrom(remoteSocket, udpReadBuf, 1500, 0, (struct sockaddr *)&outAddr, &clilen) < 0){
						//error stuff
						printf("receive from udp failed");
					}
					else{
						char* ptr;
						ptr = readHeader(udpReadBuf, &ttlUDP, &typeUDP, &senderIdUDP, &seqNumberUDP, &numLinksUDP, &numFilesUDP);
						int packetSize = 20+ numLinksUDP*sizeof(int) + numFilesUDP*9;
						int neighborId = resolvNeighbor(outAddr);
							
						if ( typeUDP == 0 ){
							int count;
							char objectName[9];
							int nodeId;
							routingEntry* re = getRoutingEntry(&routing, senderIdUDP);
							// Here we start to send Ack, no matter whether it give new updates or not
							sendAck( remoteSocket, senderIdUDP, seqNumberUDP, outAddr);
							
							// Here we need to check TTL 0 first
							if ( ttlUDP == 0){
								printf("***Deleting LSA of node %d\n", senderIdUDP);
								deleteRoutingEntry(senderIdUDP);
								printRouting(routing);
								continue;
							}
						if( re == NULL){
							printf("Receive a new Node %d\n", senderIdUDP);
							// which means this is a new node that is not my neighbor
							re = initRE(senderIdUDP, 0, "", -1 , -1, -1, 0);
							for( count = 0 ; count < numLinksUDP ; count++){
								ptr = readInt(&nodeId, ptr);
								insert(re->neighbors, &nodeId, sizeof(int));
							}
							for( count = 0 ; count < numFilesUDP ; count ++ ){
								ptr = readString(objectName, ptr);
								insert(re->objects, objectName, 9);
							}
							re->ttl = (int)ttlUDP;
							
						
							re->seqNumReceive = seqNumberUDP;
							re->numFiles = numFilesUDP;
							re->numLinks = numLinksUDP;
							re->LSACountDown = LSATimeout;	
							//printRoutingEntry(re);
							insert(&routing, re, sizeof(routingEntry));
							
							/* compute parents */
							computeParent(&routing,me);
							computeNextHops(&routing, mynodeID);
							
							forward(remoteSocket, udpReadBuf, packetSize, senderIdUDP ,neighborId);
							printRouting(routing);
							
						}else{
							
							if ( re->isNeighbor){
								re->neighborCountDown = neighborTimeout;
								if( re->isDown == 1){
									printf("Neighbor %d is back\n", re->nodeId);
									re->isDown = 0;
									me->numLinks ++;
								}
							}else{
								re->LSACountDown = LSATimeout;
							}
							
							printf("Receive LSP from %d with senNum %d, my seqReceive %d\n", senderIdUDP, seqNumberUDP, re->seqNumReceive);
				
							// here we get some updates for this node
							if( re->seqNumReceive < seqNumberUDP){
								
								// we need to clear its neighbors and objects list
								freeList(re->neighbors);
								freeList(re->objects);
								
								// update LSA accordingly
								for( count = 0 ; count < numLinksUDP ; count++ ){
									ptr = readInt(&nodeId, ptr);
									insert(re->neighbors, &nodeId, sizeof(int));
								}
								for( count = 0 ; count < numFilesUDP ; count ++ ){
									ptr = readString(objectName, ptr);
									insert(re->objects, objectName, 9);
								}
								re->ttl = (int)ttlUDP;
								re->seqNumReceive = seqNumberUDP;
								re->numFiles = numFilesUDP;
								re->numLinks = numLinksUDP;
								computeParent(&routing,me);
								computeNextHops(&routing, mynodeID);
								printRouting(routing);
								// here neighbor Id is an exclusion
								forward(remoteSocket, udpReadBuf, packetSize, senderIdUDP ,neighborId);
							}
							else if(re->seqNumReceive > seqNumberUDP){
								printf("***I receive smaller seq num %d expect %d from %d \n", seqNumberUDP, re->seqNumReceive , senderIdUDP);
								int receiverId = resolvNeighbor(outAddr);
								reTransmit(remoteSocket, receiverId, senderIdUDP);
							}	
						}// end else
						
						}// end if type == 1
						// This is Ack
						else{
							// ack must come from our neighbors
							//routingEntry* myNeighbor = getRoutingEntry(&routing, neighborId);
							printf("Receive Ack from neighbor %d, seqentail number is %d\n", neighborId, seqNumberUDP);	
							removeFromResendList(senderIdUDP, neighborId);
						}
					}
					
				}
				// It is time to send advertisement
				else if(i == advTimer){
					//printf("Advertise timer fires up !\n");
					read(advTimer,buf,sizeof(buf));
					printf("Start to advertise !\n");
					// here decrease the counter for each neighbor and LSA
					countDown(remoteSocket);
					advertise( remoteSocket, me->numLinks, me->numFiles);	
					
				}
				
			   else if(i == reTranTimer ){
					read(reTranTimer,buf,sizeof(buf));
					//printf("Retran timer fires up!\n");
					doReTransmit(remoteSocket);
			    }
			  else{
					if((re = recv(i, buffer, sizeof(buffer)-1, 0)) <= 0){
						//FD_CLR(i, &master);
						//if(clientArr[i] != NULL)
							//free(clientArr[i]);
						//close_socket(i);
						//printf("Closing %d\n",i);
					}
					else{
						clientArr[i] = realloc(clientArr[i], strlen(clientArr[i]) + re + 1);
						memcpy(clientArr[i] + strlen(clientArr[i]), buffer, re+1);
						printf("reading %s\n",clientArr[i]);
						if(sscanf(clientArr[i], "%s %d %s", method, &objLength, objectName) >= 3){
							if(!strcmp(method,"GETRD")){
								if( objLength == strlen(objectName) ){
									/* search local files */
									fileEntry* fe = getFileEntry(&fileList, objectName);
									if( fe != NULL){
										sprintf(response, "OK %zd http://localhost:%d/%s", strlen(fe->path), me->serverPort, fe->path);
									}else{
										// if not in local node, we start to find remote node
										routingEntry* desti = getFileFromOther(&routing, objectName);
										if( desti != NULL){
											routingEntry* nextHop = getRoutingEntry(&routing, desti->nextHop);
											char tmp[200];
											sprintf(tmp, "http://%s:%d/rd/%d/%s",nextHop->hostName, \
											nextHop->serverPort,nextHop->localPort, objectName);
											sprintf(response, "OK %zd %s", strlen(tmp), tmp);
										}
										else{
										sprintf(response, "NOTFOUND 0 ");
										}
									}
									ssize_t tmp;
									if(( tmp = send(i, response, strlen(response), 0)) <= 0 ){
										printf("Send response failed\n");
									} 
									
									memset(method, '\0', 10);
									memset(objectName, '\0', 100);
								}
							}
							else if(!strcmp(method,"ADDFILE")){
								if(sscanf(clientArr[i], "%s %d %s %d %s", method, &objLength, objectName, &pathLength, path) >= 5){
									if( pathLength == strlen(path)){
										fileEntry* fe = initFL(objectName, path);
										insert(&fileList, fe, sizeof(fileEntry));
										insert(me->objects, objectName, 9);
										sprintf(response, "OK 0 ");
										ssize_t tmp;
										if(( tmp = send(i, response, strlen(response), 0)) <= 0 ){
											printf("Send response failed\n");
										}
										printf("response: %s\n", response);
										memset(method, '\0', 10);
										memset(objectName, '\0', 100);
										memset(path,'\0', 100);
										
										me->numFiles++;
										advertise( remoteSocket, me->numLinks, me->numFiles);
									} 
								}
							}
						}
					}
				}
			}
		}//end for
			
		//other events
			
		memset(response,'\000',256);
		memset(buf, '\0',256);
		memset(buffer, '\000', 10);
		
		
		
	}//end while
	
	
    return 1;
}
