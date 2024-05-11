/**
 * @file  mips.h
 * @copyright Copyright (c) 2024
 */

#ifndef _MIPS_H_
#define _MIPS_H_

#include "common.h"
#include "pipeline.h"

typedef struct {
  uint32_t total;
  uint32_t arithmetic;
  uint32_t logical;
  uint32_t memory;
  uint32_t control;
} InstructionCount;

typedef struct {
  uint32_t registers[32];
  uint32_t memory[MEMORY_SIZE];
  uint32_t memory_size;
  uint32_t pc;
  uint32_t clock;
  Pipeline pipeline;
  bool halt;
  InstructionCount counts;
} MIPSSim;

void init_simulator(MIPSSim *mips);
void destroy_simulator(MIPSSim *mips);
void load_memory(MIPSSim *mips, char *filename);
void fetch_stage(MIPSSim *mips);
void decode_stage(MIPSSim *mips);
void execute_stage(MIPSSim *mips);
void memory_stage(MIPSSim *mips);
void writeback_stage(MIPSSim *mips);
void process(MIPSSim *mips);

uint32_t perform_operation(uint32_t rs, uint32_t rt, Opcode opcode);
bool control_flow(MIPSSim *mips, Instruction *instr);
void adjust_pc(MIPSSim *mips);

void log_registers(MIPSSim *mips);
void log_memory(MIPSSim *mips);

#endif