#include <stdio.h>
#include "mips.h"
#include "queue.h"


int main() {
  MIPS_Init *mips = (MIPS_Init*)malloc(sizeof(MIPS_Init));
  initializeSimulator(mips);
  loadMemory("memory_image.txt", mips);
  Queue q;
  init_queue(&q);
  
  while (!mips->halt) {
    
    if (!is_queue_full(&q) && mips->pc / 4 < MEMORY_SIZE) {
        MIPS_Instruction* newInst = (MIPS_Instruction*)malloc(sizeof(MIPS_Instruction));
        newInst->instruction = mips->memory[mips->pc / 4];  // Load the instruction
        printf("%d\n", newInst->instruction);
        enqueue(&q, newInst);  // Enqueue with initial fetch stage

    }
    processQueue(&q, mips);  // Process all instructions in the queue
}


  printf("Simulation completed in %u clock cycles.\n", mips->clock);
  printf("Final PC: %u\n", mips->pc);  // Print the final PC value

  printRegisters(mips);
  printMemory(mips);
  printf("Instruction counts:\n");
  printf("Total number of instructions: %u\n", mips->totalInstructions);
  printf("Arithmetic instructions: %u\n", mips->arithmeticInstructions);
  printf("Logical instructions: %u\n", mips->logicalInstructions);
  printf("Memory access instructions: %u\n", mips->memoryAccessInstructions);
  printf("Control transfer instructions: %u\n", mips->controlTransferInstructions);

  return 0;
}
