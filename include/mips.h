/**
 * @file  mips.h
 * @copyright Copyright (c) 2024
 */

#ifndef _MIPS_H_
#define _MIPS_H_

#include "common.h"
#include "pipeline.h"

typedef enum {
  NOT_PIPED,
  PIPED_NO_FWD,
  PIPED_FWD
} Mode;

typedef struct {
  uint32_t total;
  uint32_t arithmetic;
  uint32_t logical;
  uint32_t memory;
  uint32_t control;
} InstructionCount;

typedef struct {
  uint32_t value;
  bool modified;
} Value;

typedef struct {
  Value registers[32];
  Value memory[MEMORY_SIZE];
  uint32_t memory_size;
  uint32_t pc;
  uint32_t clock;
  Pipeline pipeline;
  bool halt;
  InstructionCount counts;
  Mode mode;
} MIPSSim;

void init_simulator(MIPSSim *mips, Mode mode);
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

void print_registers(MIPSSim *mips);
void print_memory(MIPSSim *mips);

#endif