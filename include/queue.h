#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdbool.h>
#include "mips.h"  // Ensure MIPS_Instruction is defined before this file is included.

#define QUEUE_SIZE 5

typedef struct {
    MIPS_Instruction items[QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

// Function declarations
void init_queue(Queue *q);
bool is_queue_empty(const Queue *q);
bool is_queue_full(Queue *q);
bool enqueue(Queue *q, MIPS_Instruction element);
MIPS_Instruction dequeue(Queue *q);
int queue_size(Queue *q);
MIPS_Instruction peek(const Queue *q);
MIPS_Instruction peekAt(const Queue *q, int index);
void print_queue(const Queue *q);  // Use const here to match the expected usage in print_queue
void processQueue(Queue *q, MIPS_Init *mips);

#endif
