#include "queue.h"

// Function to initialize the queue
void init_queue(Queue *q) {
  q->front = 0;
  q->rear = -1;
  q->size = 0;
}

// Function to check if the queue is empty
bool is_queue_empty(Queue *q) {
  return q->size == 0;
}

// Function to check if the queue is full
bool is_queue_full(Queue *q) {
  return q->size == QUEUE_SIZE;
}

// Function to add an element to the back of the queue
bool enqueue(Queue *q, int element) {
  if (is_queue_full(q)) {
    return false;
  }
  q->rear = (q->rear + 1) % QUEUE_SIZE;
  q->items[q->rear] = element;
  q->size++;
  return true;
}

// Function to remove an element from the front of the queue
int dequeue(Queue *q) {
  if (is_queue_empty(q)) {
    printf("Queue is empty!\n");
    return -1;
  }
  int item = q->items[q->front];
  q->front = (q->front + 1) % QUEUE_SIZE;
  q->size--;
  return item;
}

// Function to get the current size of the queue
int queue_size(Queue *q) {
  return q->size;
}

// Function to return the front element of the queue without removing it
int peek(Queue *q) {
  if (is_queue_empty(q)) {
    printf("Queue is empty!\n");
    return -1;
  }
  return q->items[q->front];
}

// Function to print all elements in the queue
void print_queue(Queue *q) {
  if (is_queue_empty(q)) {
    printf("Queue is empty!\n");
    return;
  }
  int count = q->size;
  int index = q->front;
  printf("Queue elements: ");
  while (count--) {
    printf("%d ", q->items[index]);
    index = (index + 1) % QUEUE_SIZE;
  }
  printf("\n");
}
