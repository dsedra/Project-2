#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "algo.h"

void computeParent(linkedList* rTable, routingEntry* sourcep){
	linkedList que;
	que.head = NULL;
	que.tail = NULL;
	sourcep->visited = 1;
	sourcep->parent = NULL;
	
	insert(&que, sourcep, sizeof(routingEntry));
	
	while(que.head != NULL){
		routingEntry* cur = popQue(&que);
		printf("Pop node %d\n", cur->nodeId);
		/* loop through children */
		
		node* child = cur->neighbors->head;
		
		while(child != NULL){
			routingEntry* childEntry = getRoutingEntry(rTable, *(int *)child->data);
			
			if((childEntry->visited == 0) && (!childEntry->isDown)){
				childEntry->parent = cur;
				childEntry->visited = 1;
				insert(&que, childEntry,  sizeof(routingEntry));
			}
			
			child = child->next;
		}
	}
}

void computeNextHops(linkedList* rTable, int srcId){
	
	node* curr = rTable->head;
	while( curr != NULL){
		routingEntry* detination = (routingEntry*) curr->data;
		if( detination->nodeId != srcId ){
			detination->nextHop	= computeNextHop(detination, srcId);
		}
		curr = curr->next;
	}
	
}

int computeNextHop(routingEntry* destination,int srcId){
	
	routingEntry* curr  = destination;
	
	while( curr->parent->nodeId != srcId ){
		curr = curr->parent;
	}
	
	return curr->nodeId;
}

routingEntry* popQue(linkedList* que){
	node* temp = que->head;	
	
	
	/* case 1 element */
	if(que->head == que->tail){
		que->head = NULL;
		que->tail = NULL;
	}
	/* case 2 or more */
	else{
		que->head = que->head->next;
	}
	temp->next = NULL;
	free(temp);
	routingEntry* re = temp->data;
	
	return re;
}





