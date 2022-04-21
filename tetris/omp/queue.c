#include "queue.h"
#include "assert.h"

queue* create_queue(int n) {
    queue* q = (queue*) malloc(sizeof(queue));
    q->capacity = n;
    q->array = (state**) malloc(sizeof(state*) * n);
    q->front = 0;
    q->size = 0;
    q->rear = n - 1;
    return q;
}

state* front(queue *q){
    if (q->size == 0)
        assert(0);
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

void delete_queue(queue *q) {
    int n = q->capacity;
    free(q->array);
    free(q);
}