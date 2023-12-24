#pragma once
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
	int arrival_time;
	int source_port;
	int dest_port;
	struct node* next;
};

struct list* init_list();
void insert(struct list* list, int arrival_time, int source_port, int dest_port);
void delete_first(struct list* list);
int is_empty(struct list* list);
void print_List(struct list* list);
