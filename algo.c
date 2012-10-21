#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


typedef struct node_s{
	void* data;
	struct node_s *next;
}node;

typedef struct list{
	node* head;
	node* tail;
}linkedList;

typedef struct re{
	int nodeId;
	int nextHop;
	int visited;
	struct re* parent;
	linkedList* neighbors;
	linkedList* objects;

}routingEntry;

void computeParent(linkedList* rTable, routingEntry* sourcep);
routingEntry* popQue(linkedList* que);
routingEntry* getRoutingEntry( linkedList* list, int nodeId );
void insert(linkedList* list, void* data, int size);
void printRouting(linkedList list);
node* freeNode(node* n);
void computeNextHops(linkedList* rTable, int srcId);

int main(){
	linkedList rTable;
	rTable.head = NULL;
	rTable.tail = NULL;
	
	/* re1 */
	routingEntry re1;
	re1.nodeId = 1;
	re1.nextHop = -1;
	re1.visited = 0 ;
	re1.parent = NULL;
	re1.neighbors = malloc(sizeof(linkedList));
	re1.neighbors->head = NULL;
	re1.neighbors->tail = NULL;
	int* re1ne1 = malloc(sizeof(int));
	int* re1ne2 = malloc(sizeof(int));
	*re1ne1 = 2;
	*re1ne2 = 3;
	insert(re1.neighbors, re1ne1, sizeof(int));
	insert(re1.neighbors, (void* )re1ne2, sizeof(int));
	
	/* re2 */
	routingEntry re2;
	re2.nodeId = 2;
	re2.nextHop = -1;
	re2.visited = 0 ;
	re2.parent = NULL;
	re2.neighbors = malloc(sizeof(linkedList));
	re2.neighbors->head = NULL;
	re2.neighbors->tail = NULL;
	int* re2ne0 = malloc(sizeof(int));
	int* re2ne1 = malloc(sizeof(int));
	int* re2ne2 = malloc(sizeof(int));
	*re2ne0 = 1;
	*re2ne1 = 4;
	*re2ne2 = 3;
	insert(re2.neighbors, (void* )re2ne0, sizeof(int));
	insert(re2.neighbors, (void* )re2ne1, sizeof(int));
	insert(re2.neighbors, (void* )re2ne2, sizeof(int));
	
	/* re3 */
	routingEntry re3;
	re3.nodeId = 3;
	re3.nextHop = -1;
	re3.visited = 0 ;
	re3.parent = NULL;
	re3.neighbors = malloc(sizeof(linkedList));
	re3.neighbors->head = NULL;
	re3.neighbors->tail = NULL;
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
	re4.nextHop = -1;
	re4.visited = 0 ;
	re4.parent = NULL;
	re4.neighbors = malloc(sizeof(linkedList));
	re4.neighbors->head = NULL;
	re4.neighbors->tail = NULL;
	int* re4ne1 = malloc(sizeof(int));
	int* re4ne2 = malloc(sizeof(int));
	*re4ne1 = 2;
	*re4ne2 = 5;
	insert(re4.neighbors, (void* )re4ne1, sizeof(int));
	insert(re4.neighbors, (void* )re4ne2, sizeof(int));

	/* re 5 */
	routingEntry re5;
	re5.nodeId = 5;
	re5.nextHop = -1;
	re5.visited = 0 ;
	re5.parent = NULL;
	re5.neighbors = malloc(sizeof(linkedList));
	re5.neighbors->head = NULL;
	re5.neighbors->tail = NULL;
	int* re5ne0 = malloc(sizeof(int));
	int* re5ne1 = malloc(sizeof(int));
	int* re5ne2 = malloc(sizeof(int));
	*re5ne0 = 6;
	*re5ne1 = 3;
	*re5ne2 = 4;
	insert(re5.neighbors, (void* )re5ne0, sizeof(int));
	insert(re5.neighbors, (void* )re5ne1, sizeof(int));
	insert(re5.neighbors, (void* )re5ne2, sizeof(int));
	
	/* re6 */
	routingEntry re6;
	re6.nodeId = 6;
	re6.nextHop = -1;
	re6.visited = 0 ;
	re6.parent = NULL;
	re6.neighbors = malloc(sizeof(linkedList));
	re6.neighbors->head = NULL;
	re6.neighbors->tail = NULL;
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
	
	
	routingEntry* source = getRoutingEntry(&rTable, 1);
	computeParent(&rTable, source);
		
	computeNextHops(&rTable, 1); 	
	printRouting(rTable);
	
	return 0;
}

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
			
			if(childEntry->visited == 0){
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
	return (routingEntry *)temp->data;
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

void insert(linkedList* list, void* data, int size){
	
	node* n;
	n = malloc( sizeof(node) );
	n->data = malloc(size);
	
	memcpy(n->data, data, size);
	n->next = NULL;
	
	if ( list->head == NULL ){
		list->head = n;
		list->tail = n;
	}else{
		list->tail->next = n;
		list->tail = n;
	}
}

void printRouting(linkedList list){
    node* currentp = list.head;
    
    while(currentp != NULL){
        routingEntry* entryp = (routingEntry *)currentp->data;
		printf("Node: %d, nextHop: %d ", entryp->nodeId, entryp->nextHop);
		if ( entryp->parent != NULL ){
			printf("parent: node %d\n", entryp->parent->nodeId);
		}else{
			printf("parent: unknwon\n");
		}
	
		node* curr;
		curr = entryp->neighbors->head;
		while( curr != NULL){
			int* nb = (int*) curr->data;
			
			routingEntry* re = getRoutingEntry(&list, *nb);
			char* status = "LIVE";
			
			printf("---Neighbor %d\n", *nb);
						
			curr = curr->next;
		}
        currentp = currentp->next;
    }
    
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


