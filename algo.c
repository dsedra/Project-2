#include <stdlib.h>
#include <stdio.h>
#include "linkedList.h"

void computeParent(linkedList rTable, routingEntry* sourcep);
routingEntry* popQue(linkedList* que);


int main(){
	linkedList rTable;
	
	/* re1 */
	routingEntry re1;
	re1.nodeId = 1;
	re1.neighbors = malloc(sizeof(linkedList));
	int* re1ne1 = malloc(sizeof(int));
	int* re1ne2 = malloc(sizeof(int));
	*re1ne1 = 2;
	*re1ne2 = 3;
	insert(re1.neighbors, (void* )re1ne1, sizeof(int));
	insert(re1.neighbors, (void* )re1ne2, sizeof(int));
	
	/* re2 */
	routingEntry re2;
	re2.nodeId = 2;
	re2.neighbors = malloc(sizeof(linkedList));
	int* re2ne1 = malloc(sizeof(int));
	int* re2ne2 = malloc(sizeof(int));
	*re2ne1 = 4;
	*re2ne2 = 3;
	insert(re1.neighbors, (void* )re2ne1, sizeof(int));
	insert(re1.neighbors, (void* )re2ne2, sizeof(int));
	
	/* re3 */
	routingEntry re3;
	re3.nodeId = 3;
	re3.neighbors = malloc(sizeof(linkedList));
	int* re3ne1 = malloc(sizeof(int));
	int* re3ne2 = malloc(sizeof(int));
	int* re3ne3 = malloc(sizeof(int));
	*re3ne1 = 1;
	*re3ne2 = 2;
	*re3ne3 = 5;
	insert(re3.neighbors, (void* )re3ne1, sizeof(int));
	insert(re3.neighbors, (void* )re3ne2, sizeof(int));
	insert(re3.neighbors, (void* )re3ne3, sizeof(int));
	
	/* re4 */
	routingEntry re4;
	re4.nodeId = 4;
	re4.neighbors = malloc(sizeof(linkedList));
	int* re4ne1 = malloc(sizeof(int));
	int* re4ne2 = malloc(sizeof(int));
	*re4ne1 = 2;
	*re4ne2 = 5;
	insert(re4.neighbors, (void* )re4ne1, sizeof(int));
	insert(re4.neighbors, (void* )re4ne2, sizeof(int));

	/* re 5 */
	routingEntry re5;
	re5.nodeId = 5;
	re5.neighbors = malloc(sizeof(linkedList));
	int* re5ne1 = malloc(sizeof(int));
	int* re5ne2 = malloc(sizeof(int));
	*re5ne1 = 3;
	*re5ne2 = 4;
	insert(re5.neighbors, (void* )re5ne1, sizeof(int));
	insert(re5.neighbors, (void* )re5ne2, sizeof(int));
	
	/* re6 */
	routingEntry re6;
	re6.nodeId = 6;
	re6.neighbors = malloc(sizeof(linkedList));
	int* re6ne1 = malloc(sizeof(int));
	*re6ne1 = 5;
	insert(re6.neighbors, (void* )re6ne1, sizeof(int));

	
	/* insert entries */
	insert(&rTable, &re1, sizeof(routingEntry));
	insert(&rTable, &re2, sizeof(routingEntry));
	insert(&rTable, &re3, sizeof(routingEntry));
	insert(&rTable, &re4, sizeof(routingEntry));
	insert(&rTable, &re5, sizeof(routingEntry));
	insert(&rTable, &re6, sizeof(routingEntry));
	
	
	return 0;
}

void computeParent(linkedList rTable, routingEntry* sourcep){
	linkedList que;
	sourcep->visited = 1;
	sourcep->parent = NULL;
	
	insert(&que, sourcep, sizeof(routingEntry));
	
	while(que.head != NULL){
		routingEntry* cur = popQue(&que);
		/* loop through children */
		
		node* child = cur->neighbors->head;
		
		while(child != NULL){
			routingEntry* childEntry = getRoutingEntry(&rTable, *(int *)child->data);
			
			if(childEntry->visited == 0){
				childEntry->parent = cur;
				insert(&que, childEntry,  sizeof(routingEntry));
				childEntry->visited = 1;
			}
			
			child = child->next;
		}
		
	}
}

routingEntry* popQue(linkedList* que){
	node* temp = que->head;
	temp->next = NULL;
	
	/* case 1 element */
	if(que->head == que->tail){
		que->head = NULL;
		que->tail = NULL;
	}
	/* case 2 or more */
	else{
		que->head = que->head->next;
	}
	
	return (routingEntry *)temp;
}
