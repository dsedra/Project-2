#ifndef ALGO_H
#define ALGO_H
#include "linkedList.h"

void computeParent(linkedList* rTable, routingEntry* sourcep);
routingEntry* popQue(linkedList* que);
void computeNextHops(linkedList* rTable, int srcId);
int computeNextHop(routingEntry* destination,int srcId);

#endif