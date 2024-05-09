#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define QUEUE_SIZE 100

typedef struct {
    void* items[QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

void init_queue(Queue *q);
bool is_queue_empty(Queue *q);
bool is_queue_full(Queue *q);
bool enqueue(Queue *q, void *element);
void* dequeue(Queue *q);
int queue_size(Queue *q);
void* peek(Queue *q);
void* peek_at(Queue *q, int index);
void print_queue(Queue *q, void (*print_func)(void*));

#endif // QUEUE_H
