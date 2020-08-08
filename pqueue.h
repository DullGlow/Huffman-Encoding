#ifndef P_QUEUE_INCLUDE
#define P_QUEUE_INCLUDE

#include "huffmantypes.h"
#include <stdlib.h>

typedef struct PQ_Node { 
	Node *data; 
	
	int priority; 
	
	struct PQ_Node *next; 
	
} PQ_Node;

PQ_Node* pq_newNode(Node* d, int p);

Node* pq_pop(PQ_Node** head);

void pq_push(PQ_Node** head, Node* d, int p);

int pq_isEmpty(PQ_Node** head);

#endif