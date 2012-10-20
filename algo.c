#include <stdlib>
#include <stdio>
#include "linkedList.h"

int main(){
	linkedList rTable;
	
	routingEntry re1;
	re1.nodeId = 1;
	re1.neighbors = malloc(sizeof(linkedList));
	insert(&re1.neighbors, 2);
	insert(&re1.neibhbors, 3);
	
	routingEntry re2;
	re2.nodeId = 2;
	re2.neighbors = malloc(sizeof(linkedList));
	insert(&re2.neighbors, 4);
	insert(&re2.neibhbors, 3);
	
	routingEntry re3;
	re3.nodeId = 3;
	re3.neighbors = malloc(sizeof(linkedList));
	insert(&re3.neighbors, 1);
	insert(&re3.neibhbors, 2);
	insert(&re3.neibhbors, 5);
	
	routingEntry re4;
	re4.nodeId = 4;
	re4.neighbors = malloc(sizeof(linkedList));
	insert(&re4.neibhbors, 2);
	insert(&re4.neibhbors, 5);
	
	routingEntry re5;
	re5.nodeId = 5;
	re5.neighbors = malloc(sizeof(linkedList));
	insert(&re5.neighbors, 3);
	insert(&re5.neibhbors, 4);
	
	routingEntry re6;
	re6.nodeId = 6;
	re6.neighbors = malloc(sizeof(linkedList));
	insert(&re6.neighbors, 5);
	
	/* insert entries */
	insert(&rTable, re1);
	insert(&rTable, re2);
	insert(&rTable, re3);
	insert(&rTable, re4);
	insert(&rTable, re5);
	insert(&rTable, re6);
	
	
	return 0;
}

void computeParent(linkedList rTable, routingEntry* sourcep){
	linkedList que;
	sourcep->visited = 1;
	sourcep->parent = NULL;
	
	insert(&que, sourcep, sizeof(routingEntry));
	
	while(que.head != NULL){
		node* cur = popQue(que);
		/* loop through children */
		node* child = cur->neighbors->head;
		while(child != NULL){
			routingEntry* childDatap = (routingEntry *)child->data;
			if(childDatap->visited == 0){
				childDatap->parent = (routingEntry *)cur->data;
				insert(&rTable, child,  sizeof(node));
				childDatap->visited = 1;
			}
			
		}
	}
	routingEntry* curp = source.neighbors.head;
}

node* popQue(linkedList* que){
	node* temp = que->head;
	temp->next = NULL;
	que->head = que->head->next;
	
	return temp;
}
