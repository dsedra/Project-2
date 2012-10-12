#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedList.h"

void BFS( routingEntry* source){
	
	linkedList* queue;
	queue = malloc(sizeof(linkedList));
	source->visited = 1;
	source->parent = NULL;
	
	insert(queue, source, sizeof(routingEntry));
	
	while( queue->head != NULL ){
		
		routingEntry* poped = pop( queue );
		linkedList* neighbors = poped->neighbors;
		node* curr = neighbors->head;
		
		while( curr != NULL ){
			
			routingEntry* re = (routingEntry*)curr->data; 
			
			routingEntry* item = getRoutingEntry(&routing, re->nodeId);
			// not visited before
			if( item->visited == 0){
				item->visited = 1;
				item->parent = poped;
				insert(queue, item, sizeof(routingEntry));
			}
			curr = curr->next;
		}	
	}
}

int nextHop( routingEntry* source, routingEntry* dest ){
	
	routingEntry* curr = dest;
	int nodeId = curr->nodeId;
	
	while ( curr->parent != source ){
		curr = curr->parent;
		nodeId = curr->nodeId;	
	}
	
	return nodeId;
}
