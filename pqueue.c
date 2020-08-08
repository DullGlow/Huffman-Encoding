#include "pqueue.h"

PQ_Node* pq_newNode(Node* d, int p) {
	PQ_Node* temp = (PQ_Node*)malloc(sizeof(PQ_Node));
	temp->data = d;
	temp->priority = p;
	temp->next = NULL;

	return temp;
}

Node* pq_pop(PQ_Node** head) {
	Node* result = (*head)->data;
	PQ_Node* temp = *head;
	(*head) = (*head)->next;
	free(temp);
	return result;
}

void pq_push(PQ_Node** head, Node* d, int p) {
	if ((*head) == NULL) {
		*head = pq_newNode(d, p);
		return;
	}

	PQ_Node* start = (*head);

	PQ_Node* temp = pq_newNode(d, p);

	if ((*head)->priority > p) {
		temp->next = *head;
		(*head) = temp;
	}
	else {
		while (start->next != NULL && start->next->priority < p) {
			start = start->next;
		}

		temp->next = start->next;
		start->next = temp;
	}
}

int pq_isEmpty(PQ_Node** head) {
	return (*head) == NULL;
}