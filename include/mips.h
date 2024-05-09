#ifndef _MIPS_H_
#define _MIPS_H_
#include "queue.h"
#define MEMORY_SIZE 4096  // Assuming memory size for simplicity

typedef enum {
  IF,
  ID,
  EX,
  MEM,
  WB,
  DONE
} PipelineStage;

typedef struct {
  unsigned int instruction;
  PipelineStage stage;
  unsigned int opcode, rs, rt, rd, imm;
  int result;  // For storing results from execution
  int writeBackAddr;
  int memValue;
} MIPS_Instruction;

typedef struct {
  int registers[32];
  unsigned int memory[MEMORY_SIZE];
  unsigned int pc;
  unsigned int clock;
  int halt;
  MIPS_Instruction pipeline;
  unsigned int writtenMemory[MEMORY_SIZE];
  unsigned int totalInstructions;
  unsigned int arithmeticInstructions;
  unsigned int logicalInstructions;
  unsigned int memoryAccessInstructions;
  unsigned int controlTransferInstructions;
} MIPS_Init;

void initializeSimulator(MIPS_Init *mips);
void loadMemory(char *filename, MIPS_Init *mips);
void fetch(MIPS_Init *mips, MIPS_Instruction *instr);
void decode(MIPS_Init *mips, MIPS_Instruction *instr);
void executeInstruction(MIPS_Init *mips, MIPS_Instruction *instr);
void memoryAccess(MIPS_Init *mips, MIPS_Instruction *instr);
void writeBack(MIPS_Init *mips, MIPS_Instruction *instr);
void printRegisters(MIPS_Init *mips);
void printMemory(MIPS_Init *mips);
void controlFlow(MIPS_Instruction *instr, MIPS_Init *mips);
int performArithmeticOperation(unsigned int opcode, int rsValue, int rtValue, int immValue);
int performImmediateOperation(unsigned int opcode, int rsValue, int immValue);
void processQueue(Queue *q, MIPS_Init *mips);

#endif