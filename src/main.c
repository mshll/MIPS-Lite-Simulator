#include <stdio.h>
#include "mips.h"

int main() {
  MIPS_Init mips;
  initializeSimulator(&mips);
  loadMemory("memory_image.txt", &mips);

  while (!mips.halt) {
    fetch(&mips);
    decode(&mips);
    executeInstruction(&mips);
  }

  printf("Simulation completed in %u clock cycles.\n", mips.clock);
  printf("Final PC: %u\n", mips.pc);  // Print the final PC value

  printRegisters(&mips);
  printMemory(&mips);
  printf("Instruction counts:\n");
  printf("Total number of instructions: %u\n", mips.totalInstructions);
  printf("Arithmetic instructions: %u\n", mips.arithmeticInstructions);
  printf("Logical instructions: %u\n", mips.logicalInstructions);
  printf("Memory access instructions: %u\n", mips.memoryAccessInstructions);
  printf("Control transfer instructions: %u\n", mips.controlTransferInstructions);

  return 0;
}
