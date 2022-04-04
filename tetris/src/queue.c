#include "queue.h"

queue* create_queue(int n) {
    queue* q = (queue*) malloc(sizeof(queue));
    q->capacity = n;
    q->array = (state**) malloc(sizeof(state*) * n);
    for (int i = 0; i < n; i++) {
        q->array[i] = (state*) malloc(sizeof(state));
    }
    q->front = 0;
    q->size = 0;
    q->rear = n - 1;
    return q;
}

state* front(queue *q){
    if (q->size == 0)
        return NULL;
    return q->array[q->front];
}
void push(queue *q, state* s){
    q->rear = (q->rear + 1) % q->capacity;
    q->array[q->rear] = s;
    q->size++;
}
void pop(queue *q){
    if (q->size == 0)
        return;
    q->front = (q->front + 1) % q->capacity;
    q->size--;
}
int size(queue *q){
    return q->size;
}
