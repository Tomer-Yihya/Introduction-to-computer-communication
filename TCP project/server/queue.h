#pragma once

struct queue {
	int* itemArray;
	int front;
	int rear;
	int itemCount;
	int max_size;
};

struct queue* createQueue(int max_size);

int peek(struct queue* q);

int isEmpty(struct queue* q);

int isFull(struct queue* q);

int size(struct queue* q);

void insert(struct queue* q, int data);

int removeData(struct queue* q); //#pragma once
