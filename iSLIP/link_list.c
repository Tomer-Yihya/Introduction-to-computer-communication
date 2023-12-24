#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "link_list.h"




struct list* init_list() {
	struct list* new_list = (struct list*)malloc(sizeof(struct list));
	new_list->size = 0;
	new_list->head = NULL;
	new_list->last = NULL;
	return new_list;
}

// insert as the last node in the list
void insert(struct list* list, int arrival_time, int source_port, int dest_port) {

	// create a node with job id = job_id, gen_time = time, end_prog_time = time
	struct node* new_node = (struct node*)malloc(sizeof(struct node));
	if (new_node == NULL) {
		return;
	}
	new_node->arrival_time = arrival_time;
	new_node->source_port = source_port;
	new_node->dest_port = dest_port;

	// if the list is empty
	if (list->size == 0) {
		list->head = new_node;
		list->last = new_node;
		list->head->next = list->last;
		list->last->next = list->head;
		list->size++;
		return;
	}

	// else insert new_node at the end
	list->last->next = new_node;    // insert new_node after last
	list->last = new_node;          // define new_node as the last list node
	list->last->next = list->head;  // the new_node next pointer is now head
	list->size++;
}

// delete the first node in the list 
void delete_first(struct list* list) {

	if (list->size == 0) {
		return;
	}
	struct node* old_head = list->head;
	struct node* new_head = old_head->next;
	struct node* last = list->last;

	list->head = new_head;
	last->next = new_head;
	free(old_head);
	list->size--;

	if (list->size == 0) {
		list->head = NULL;
		list->last = NULL;
	}
}


int is_empty(struct list* list) {
	return list->size == 0;
}


/*void print_List(struct list* list) {
	struct node* p = list->head;
	printf("List data:\n");

	//start from the beginning
	while (p != NULL) {
		printf("job_id = %d , gen_time = %Lf , end_prog_time = %Lf\n",	p->job_id, (long double)p->gen_time / CLOCKS_PER_SEC, (long double)p->end_prog_time / CLOCKS_PER_SEC);
		if (list->last == p) {
			break;
		}
		p = p->next;
	}
	printf("\n");
}*/
