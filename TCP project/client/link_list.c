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
void insert(struct list* list, int job_id, clock_t time) {

	// create a node with job id = job_id, gen_time = time, end_prog_time = time
	struct node* new_node = (struct node*)malloc(sizeof(struct node));
	if (new_node == NULL) {
		return;
	}
	new_node->job_id = job_id;
	new_node->gen_time = time;
	new_node->end_prog_time = time;
	new_node->total_time = time - time;

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

// delete the nod with job_id from thr list 
void delete_job_id(struct list* list, int job_id_to_delete) {

	if (list->size == 0) {
		return;
	}

	struct node* temp = list->head;
	struct node* prev = list->last;

	// edge case => job_id is in the list head node
	if (temp->job_id == job_id_to_delete) {
		prev->next = temp->next;
		list->head = prev->next;
		free(temp);
		list->size--;
	}

	// Find the job_id to be deleted
	for (int i = 0; i < list->size; i++) {
		if (temp != NULL) {
			if (temp->job_id == job_id_to_delete) {
				break;
			}
			prev = temp;
			temp = temp->next;
		}
	}
	// If the job_id is not present
	if (temp == NULL)
		return;

	// Remove the node
	if (temp == list->last) {
		list->last = prev;
	}
	prev->next = temp->next;
	free(temp);
	list->size--;
}

// return the node with the job_id or NULL
struct node* search_job_id(struct list* list, int job) {
	struct node* temp = list->head;
	while (temp != NULL) {
		if (temp->job_id == job) {
			return temp;
		}
		if (list->last == temp) {
			break;
		}
		temp = temp->next;
	}
	return NULL;
}

void update_end_prog_time(struct list* list, int job, clock_t end_time , int status) {
	struct node* temp = search_job_id(list, job);
	if (temp == NULL) {
		return;
	}
	temp->status = status;
	if (status) {
		temp->end_prog_time = end_time;
		temp->total_time = (end_time - temp->gen_time);
	}
	else {
		temp->end_prog_time = 0;
		temp->total_time = 0;
	}
}

void print_List(struct list* list) {
	struct node* p = list->head;
	printf("List data:\n");

	//start from the beginning
	while (p != NULL) {
		printf("job_id = %d , gen_time = %Lf , end_prog_time = %Lf\n",
			p->job_id, (long double)p->gen_time / CLOCKS_PER_SEC, (long double)p->end_prog_time / CLOCKS_PER_SEC);
		if (list->last == p) {
			break;
		}
		p = p->next;
	}
	printf("\n");
}
