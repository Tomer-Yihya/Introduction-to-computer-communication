#ifndef LINK_LIST_H
#define LINK_LIST_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


struct list {
	int size;
	struct node* head;
	struct node* last;
};

struct node {
	int job_id;
	clock_t gen_time;
	clock_t end_prog_time;
	clock_t total_time;
	struct node* next;
	int status;
};

struct list* init_list();
void insert(struct list* list, int job_id, clock_t time);
void delete_job_id(struct list* list, int job_id_to_delete);
void update_end_prog_time(struct list* list, int job, clock_t end_time, int status);
void print_List(struct list* list);
#endif
