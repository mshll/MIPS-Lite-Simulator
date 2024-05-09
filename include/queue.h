#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdbool.h>
#include <stdio.h>

#define QUEUE_SIZE 100

// Queue structure definition
typedef struct {
  int items[QUEUE_SIZE];
  int front;
  int rear;
  int size;
} Queue;

// Function declarations
void init_queue(Queue *q);
bool is_queue_empty(Queue *q);
bool is_queue_full(Queue *q);
bool enqueue(Queue *q, int element);
int dequeue(Queue *q);
int queue_size(Queue *q);
int peek(Queue *q);
void print_queue(Queue *q);

#endif
