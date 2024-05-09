#include "mips.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initializeSimulator(MIPS_Init *mips) {
  memset(mips->registers, 0, sizeof(mips->registers));          // memset() sets all registers to zero
  memset(mips->memory, 0, sizeof(mips->memory));                // memset() sets all memory to zero
  memset(mips->writtenMemory, 0, sizeof(mips->writtenMemory));  // setting the flags for our array
  mips->pc = 0;                                                 // Initializing PC
  mips->clock = 0;                                              // Initializing clock
  mips->halt = 0;                                               // // Initializing halt flag to zero
  mips->totalInstructions = 0;                                  // Initializing total instruction counter to zero
  mips->arithmeticInstructions = 0;                             // Initializing artithmetic instruction counter to zero
  mips->logicalInstructions = 0;                                // Initializing logical instruction counter to zero
  mips->memoryAccessInstructions = 0;                           // Initializing memory instruction counter to zero
  mips->controlTransferInstructions = 0;                        // Initializing trnsfer instruction counter to zero
}

void loadMemory(char *filename, MIPS_Init *mips) {  // Loading all the .txt file to memory, not sure if thats right?
  FILE *file = fopen(filename, "r");                // Error handling
  if (!file) {
    perror("Failed to open file");
    return;
  }
  char line[10];
  int i = 0;
  while (fgets(line, sizeof(line), file) && i < MEMORY_SIZE) {
    mips->memory[i] = strtol(line, NULL, 16);
    i++;
  }
  fclose(file);
}

// everything below not finished
void fetch(MIPS_Init *mips) {
  if (mips->pc / 4 < MEMORY_SIZE && !mips->halt) {
    mips->pipeline[IF].instruction = mips->memory[mips->pc / 4];
    mips->pipeline[IF].stage = ID;  // Move to decode
    mips->pc += 4;                  // Increment PC after each fetch
    mips->clock++;                  // Clock increment at fetch
  }
}

void decode(MIPS_Init *mips) {
  if (!mips->halt) {
    mips->opcode = (mips->instruction >> 26) & 0x3F;
    mips->rs = (mips->instruction >> 21) & 0x1F;
    mips->rt = (mips->instruction >> 16) & 0x1F;
    mips->rd = (mips->instruction >> 11) & 0x1F;
    mips->imm = (short)(mips->instruction & 0xFFFF);  // Sign-extension
    mips->clock++;
  }
}
void executeInstruction(MIPS_Init *mips) {
  if (mips->halt) return;
  int memValue = 0;
  int writeBackAddr = -1;
  mips->totalInstructions++;  // Increment total instructions counter

  switch (mips->opcode) {
    case 0x00:  // ADD
      mips->registers[mips->rd] = mips->registers[mips->rs] + mips->registers[mips->rt];
      mips->arithmeticInstructions++;
      break;
    case 0x01:  // ADDI
      mips->registers[mips->rt] = mips->registers[mips->rs] + mips->imm;
      mips->arithmeticInstructions++;
      break;
    case 0x02:  // SUB
      mips->registers[mips->rd] = mips->registers[mips->rs] - mips->registers[mips->rt];
      mips->arithmeticInstructions++;
      break;
    case 0x03:  // SUBI
      mips->registers[mips->rt] = mips->registers[mips->rs] - mips->imm;
      mips->arithmeticInstructions++;
      break;
    case 0x04:  // MUL
      mips->registers[mips->rd] = mips->registers[mips->rs] * mips->registers[mips->rt];
      mips->arithmeticInstructions++;
      break;
    case 0x05:  // MULI
      mips->registers[mips->rt] = mips->registers[mips->rs] * mips->imm;
      mips->arithmeticInstructions++;
      break;
    case 0x06:  // OR
      mips->registers[mips->rd] = mips->registers[mips->rs] | mips->registers[mips->rt];
      mips->logicalInstructions++;
      break;
    case 0x07:  // ORI
      mips->registers[mips->rt] = mips->registers[mips->rs] | mips->imm;
      mips->logicalInstructions++;
      break;
    case 0x08:  // AND
      mips->registers[mips->rd] = mips->registers[mips->rs] & mips->registers[mips->rt];
      mips->logicalInstructions++;
      break;
    case 0x09:  // ANDI
      mips->registers[mips->rt] = mips->registers[mips->rs] & mips->imm;
      mips->logicalInstructions++;
      break;
    case 0x0A:  // XOR
      mips->registers[mips->rd] = mips->registers[mips->rs] ^ mips->registers[mips->rt];
      mips->logicalInstructions++;
      break;
    case 0x0B:  // XORI
      mips->registers[mips->rt] = mips->registers[mips->rs] ^ mips->imm;
      mips->logicalInstructions++;
      break;
    case 0x0C:                                                               // LDW
      memValue = mips->memory[(mips->registers[mips->rs] + mips->imm) / 4];  // This is weird because if you remove the "/4" STW won't work :/
      writeBackAddr = mips->rt;
      mips->memoryAccessInstructions++;
      break;
    case 0x0D:                                                                                                             // STW
      printf("STW: Storing %u at memory index %u\n", mips->registers[mips->rt], (mips->registers[mips->rs] + mips->imm));  // Debug print
      mips->writtenMemory[(mips->registers[mips->rs] + mips->imm)] = 1;
      mips->memory[(mips->registers[mips->rs] + mips->imm)] = mips->registers[mips->rt];
      mips->memoryAccessInstructions++;
      break;

    case 0x0E:  // BZ
      mips->controlTransferInstructions++;
      if (mips->registers[mips->rs] == 0) mips->pc = mips->pc + 4 * mips->imm - 4;
      break;
    case 0x0F:  // BEQ
      mips->controlTransferInstructions++;
      if (mips->registers[mips->rs] == mips->registers[mips->rt]) mips->pc = mips->pc + 4 * mips->imm - 4;
      break;
    case 0x10:  // JR
      mips->controlTransferInstructions++;
      mips->pc = mips->registers[mips->rs];
      break;
    case 0x11:  // HALT
      mips->controlTransferInstructions++;
      mips->halt = 1;  // Set halt flag to stop simulation
      return;
    default:
      printf("Unknown opcode: %u\n", mips->opcode);
  }

  if (writeBackAddr != -1) {  // Write-back operation for LDW, not sure if this is how we implement it

    mips->registers[writeBackAddr] = memValue;
  }
  mips->clock++;
}
void printRegisters(MIPS_Init *mips) {
  printf("Register Values:\n");
  for (int i = 0; i < 32; i++) {
    if (mips->registers[i] != 0) {  // Only print non-zero registers to minimize clutter
      printf("R%d: %d\n", i, mips->registers[i]);
    }
  }
}

void printMemory(MIPS_Init *mips) {
  printf("Written Memory Contents:\n");
  for (int i = 0; i < MEMORY_SIZE; i++) {
    if (mips->writtenMemory[i]) {  // Only print memory cells that have been written to
      printf("Memory[%d]: %u\n", i, mips->memory[i]);
    }
  }
}
