#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>
#include "linkedList.h"
#include "packet.h"

#define mynode1Conf "own.conf"
#define mynodeID 50
#define mynode1Files "node1.files"

#define myroutingPort 6005
#define mylocalPort 5000
#define myserverPort 8080

#define max(a,b) ((a>b)?a:b)

linkedList fileList;
linkedList routing;
linkedList reSendList;
int seqNumSend;

int advCycle = 10000 ;
int retranCycle = 120 ;

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
	seqNumSend = 1;
	
	
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
    routingEntry* usp = initRE(mynodeID, 0,"127.0.0.1",
                           myroutingPort,mylocalPort,myserverPort, 0);
    
    insert(&routing, usp, sizeof(routingEntry));
       

    while(fgets(buf,sizeof(buf),conf)){
     int nodeID,routingPort,localPort,serverPort; 
     char hostName[200]; 	
        if(sscanf(buf,"%d %s %d %d %d",&nodeID,hostName,&routingPort,&localPort,&serverPort) < 5){
            printf("Couldn't parse config file\n");
            break;
        }
        
        neighbor = initRE(nodeID,0,hostName,routingPort,localPort,serverPort, 1);
        
        insert(&routing, neighbor, sizeof(routingEntry));
		// might not add the struct but add nodeID
		insert(usp->neighbors, &nodeID, sizeof(int));
		numNeighbors++; 
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
		numFiles++;
   	}	
	
    fclose(conf);
    fclose(files);
       
    printRouting(routing);
	printFile(fileList);	
	
	// create a TCP socket, and listen on it
	localSocket = socket(PF_INET, SOCK_STREAM, 0);
	cgiAddr.sin_family = AF_INET;
    cgiAddr.sin_port = htons(mylocalPort);
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
	
	printf("local socket is: %d\n", localSocket);
	
	// create a UDP socket
    remoteSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(myroutingPort);
    udpAddr.sin_addr.s_addr = INADDR_ANY;
    udpAddrLength = sizeof(udpAddr);
    
    if(bind(remoteSocket, (struct sockaddr *) &udpAddr, udpAddrLength) < 0){
        close_socket(remoteSocket);
        fprintf(stderr, "Binding remote socket failed.\n");
        return EXIT_FAILURE;
    }
	
	int advTimer = runTimer(advCycle);
	
	FD_SET(remoteSocket, &master);
	FD_SET(localSocket, &master);
	FD_SET(advTimer, &master);
	fdmax = advTimer;
	
	while(1){
		sockList = master;
		int i;
		printf("before select\n");
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
					if((re = recvfrom(remoteSocket, udpReadBuf, 1500, 0, (struct sockaddr *)&outAddr, &clilen)) < 0){
						//error stuff
						printf("receive from udp failed");
					}
					else{
						char* ptr;
						ptr = readHeader(udpReadBuf, &ttlUDP, &typeUDP, &senderIdUDP, &seqNumberUDP, &numLinksUDP, &numFilesUDP);
						routingEntry* re = getRoutingEntry(&routing, senderIdUDP);
						
						int packetSize = 20+ numLinksUDP*sizeof(int) + numFilesUDP*9;
						
						if ( typeUDP == 1 ){
							int count;
							char objectName[9];
							int nodeId;
						if( re == NULL){
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
							printf("Receive a new Node");
							printRoutingEntry(re);
							insert(&routing, re, sizeof(routingEntry));
							int neighborId = resolvNeighbor(outAddr);
							forward(remoteSocket, udpReadBuf, packetSize, neighborId);
							printRouting(routing);
							
						}else{
							// here we get some updates for this node
							printf("Sender's id: %d\n", senderIdUDP);
							printf("ttlUDP: %d\n", (int)ttlUDP);
							
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
								printRoutingEntry(re);
								int neighborId = resolvNeighbor(outAddr);
								forward(remoteSocket, udpReadBuf, packetSize, neighborId);
							}	
						}// end else
						}// end if type == 1
						// This is Ack
						else{
							re->seqNumAck++;
							// after receive this, we may clear 
							// the buffer that contains the deferred information
							
						}
					}
					
				}
				// It is time to send advertisement
				else if(i == advTimer){
					printf("Timer fires up !\n");
					read(advTimer,buf,sizeof(buf));
					printf("Start to advertise !\n");
					advertise( remoteSocket, mynodeID, numNeighbors, numFiles);				
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
									fileEntry* fe = getFileEntry(&fileList, objectName);
									if( fe != NULL){
										sprintf(response, "OK %zd http://localhost:%d/%s", strlen(fe->path), myserverPort, fe->path);
									}else{
										sprintf(response, "NOTFOUND 0 ");
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
										sprintf(response, "OK 0 ");
										ssize_t tmp;
										if(( tmp = send(i, response, strlen(response), 0)) <= 0 ){
											printf("Send response failed\n");
										}
										printf("response: %s\n", response);
										memset(method, '\0', 10);
										memset(objectName, '\0', 100);
										memset(path,'\0', 100);
										numFiles++;
										
										//routingEntry* us = getRoutingEntry(&routing,mynodeID);
										
										seqNumSend++;
										advertise( remoteSocket, mynodeID, numNeighbors, numFiles);
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
