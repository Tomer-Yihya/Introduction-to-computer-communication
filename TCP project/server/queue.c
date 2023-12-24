#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

struct queue * createQueue(int max_size) {
	struct queue* q = (struct queue*)calloc(1, sizeof(struct queue));
	q->itemArray = calloc(max_size, sizeof(int));
	q->front = 0;
	q->rear = -1;
	q->itemCount = 0;
	q->max_size = max_size;
}

int peek(struct queue* q) {
	return q->itemArray[q->front];
}

int isEmpty(struct queue* q) {
	return q->itemCount <= 0;
}

int isFull(struct queue* q) {
	return q->itemCount == q->max_size;
}

int size(struct queue* q) {
	return q->itemCount;
}

void insert(struct queue* q, int data) {

	if (!isFull(q)) {
		if (q->rear == q->max_size - 1) {
			q->rear = -1;
		}
		q->itemArray[++q->rear] = data;
		q->itemCount++;
	}
}

int removeData(struct queue* q) {
	int data = q->itemArray[q->front++];
	if (q->front == q->max_size) {
		q->front = 0;
	}

	q->itemCount--;
	return data;
}
