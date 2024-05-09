#include "queue.h"

void init_queue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

bool is_queue_empty(Queue *q) {
    return q->size == 0;
}

bool is_queue_full(Queue *q) {
    return q->size == QUEUE_SIZE;
}

bool enqueue(Queue *q, void *element) {
    if (is_queue_full(q)) {
        return false;
    }
    q->rear = (q->rear + 1) % QUEUE_SIZE;
    q->items[q->rear] = element;
    q->size++;
    return true;
}

void* dequeue(Queue *q) {
    if (is_queue_empty(q)) {
        printf("Queue is empty!\n");
        return NULL;
    }
    void *item = q->items[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->size--;
    return item;
}

int queue_size(Queue *q) {
    return q->size;
}

void* peek(Queue *q) {
    if (is_queue_empty(q)) {
        printf("Queue is empty!\n");
        return NULL;
    }
    return q->items[q->front];
}

void* peek_at(Queue *q, int index) {
    if (index < 0 || index >= q->size) {
        printf("Invalid index!\n");
        return NULL;
    }
    return q->items[(q->front + index) % QUEUE_SIZE];
}

void print_queue(Queue *q, void (*print_func)(void*)) {
    if (is_queue_empty(q)) {
        printf("Queue is empty!\n");
        return;
    }
    int count = q->size;
    int index = q->front;
    printf("Queue elements: ");
    while (count--) {
        print_func(q->items[index]);
        index = (index + 1) % QUEUE_SIZE;
    }
    printf("\n");
}
