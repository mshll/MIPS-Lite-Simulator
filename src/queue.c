#include "queue.h"
#include "mips.h"

const MIPS_Instruction MIPS_INVALID_INSTR = {0};

void init_queue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

bool is_queue_empty(const Queue *q) {
    return q->size == 0;
}

bool is_queue_full(Queue *q) {
    return q->size == QUEUE_SIZE;
}

bool enqueue(Queue *q, MIPS_Instruction element) {
    if (is_queue_full(q)) {
        return false;
    }
    q->rear = (q->rear + 1) % QUEUE_SIZE;
    q->items[q->rear] = element;
    q->size++;
    return true;
}

MIPS_Instruction dequeue(Queue *q) {
    if (is_queue_empty(q)) {
        return MIPS_INVALID_INSTR;
    }
    MIPS_Instruction item = q->items[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->size--;
    return item;
}

int queue_size(Queue *q) {
    return q->size;
}

MIPS_Instruction peek(const Queue *q) {
    if (is_queue_empty(q)) {
        return MIPS_INVALID_INSTR;
    }
    return q->items[q->front];
}

MIPS_Instruction peekAt(const Queue *q, int index) {
    if (index < 0 || index >= q->size) {
        return MIPS_INVALID_INSTR;
    }
    int realIndex = (q->front + index) % QUEUE_SIZE;
    return q->items[realIndex];
}

void processQueue(Queue *q, MIPS_Init *mips) {
    int numProcessed = q->size;
    for (int i = 0; i < numProcessed; i++) {
        int index = (q->front + i) % QUEUE_SIZE;
        MIPS_Instruction *instr = &q->items[index];
        switch (instr->stage) {
            case IF:
                fetch(mips, instr);
                instr->stage = ID;
                break;
            case ID:
                decode(mips, instr);
                instr->stage = EX;
                break;
            case EX:
                executeInstruction(mips, instr);
                instr->stage = MEM;
                break;
            case MEM:
                memoryAccess(mips, instr);
                instr->stage = WB;
                break;
            case WB:
                writeBack(mips, instr);
                instr->stage = DONE;
                break;
            case DONE:
                break;
        }
    }
    while (!is_queue_empty(q) && q->items[q->front].stage == DONE) {
        dequeue(q);
    }
    mips->clock++;
}

void print_queue(const Queue *q) {
    printf("Queue Contents:\n");
    for (int i = 0; i < q->size; i++) {
        int index = (q->front + i) % QUEUE_SIZE;
        printf("%d ", q->items[index].opcode);  // Assuming `opcode` is an integer member of MIPS_Instruction
    }
    printf("\n");
}
