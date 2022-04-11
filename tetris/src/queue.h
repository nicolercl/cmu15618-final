#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "bfs.h"

/*
 Circular queue
*/
typedef struct {
    int front;
    int rear;
    int size;
    unsigned capacity;
    state** array;
} queue;

queue* create_queue();
state* front(queue *q);
void push(queue *q, state* s);
void pop(queue *q);
int size(queue *q);
void delete_queue(queue *q);

#endif // QUEUE_H
