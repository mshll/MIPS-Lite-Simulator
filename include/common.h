#ifndef __COMMON_H__
#define __COMMON_H__

/*** includes ***/
#include <getopt.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*** macro(s), enum(s), struct(s) ***/
#ifdef DEBUG
#define LOG_DEBUG(format, ...) printf("%s:%d: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(...) /*** expands to nothing ***/
#define LOG(...)       /*** expands to nothing ***/
#endif

#define NUM_STAGES 5
#define MEMORY_SIZE 1024
#define INSTR_MASK 0x1F

typedef enum {
  IF,   // Instruction Fetch
  ID,   // Instruction Decode
  EX,   // Execute
  MEM,  // Memory Access
  WB,   // Write Back
  DONE  // Done
} PipelineStage;

typedef enum {
  ADD = 0x00,   // ADD Rd, Rs, Rt
  ADDI = 0x01,  // ADDI Rt, Rs, Imm
  SUB = 0x02,   // SUB Rd, Rs, Rt
  SUBI = 0x03,  // SUBI Rt, Rs, Imm
  MUL = 0x04,   // MUL Rd, Rs, Rt
  MULI = 0x05,  // MULI Rt, Rs, Imm
  OR = 0x06,    // OR Rd, Rs, Rt
  ORI = 0x07,   // ORI Rt, Rs, Imm
  AND = 0x08,   // AND Rd, Rs, Rt
  ANDI = 0x09,  // ANDI Rt, Rs, Imm
  XOR = 0x0A,   // XOR Rd, Rs, Rt
  XORI = 0x0B,  // XORI Rt, Rs, Imm
  LDW = 0x0C,   // LDW Rt, Rs, Imm
  STW = 0x0D,   // STW Rt, Rs, Imm
  BZ = 0x0E,    // BZ Rs, x
  BEQ = 0x0F,   // BEQ Rs, Rt, x
  JR = 0x10,    // JR Rs
  HALT = 0x11   // HALT
} Opcode;

typedef enum {
  R_TYPE,
  I_TYPE_MEM,
  I_TYPE_IMM,
  J_TYPE
} InstructionType;

typedef enum {
  RS,
  RT
} ForwardTarget;

typedef struct {
  uint32_t reg;
  bool is_forwarded;
  ForwardTarget target;
} ForwardReg;

typedef struct {
  uint32_t instruction;
  PipelineStage stage;
  InstructionType type;
  Opcode opcode;
  uint8_t rs;
  uint8_t rt;
  uint8_t rd;
  int16_t imm;
  uint32_t alu_out;
  uint32_t mdr;
  ForwardReg forward_reg;
} Instruction;

#endif