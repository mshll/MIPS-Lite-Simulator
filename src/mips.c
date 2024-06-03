/**
 * @file  mips.c
 * @copyright Copyright (c) 2024
 */

#include "mips.h"
#include "common.h"
#include "pipeline.h"

/* helper functions prototypes */
void print_memory(MIPSSim *mips);
void print_registers(MIPSSim *mips);
void correct_pc(MIPSSim *mips);
void check_hazards(MIPSSim *mips, Instruction *instr);

/**
 * @brief Initialize the MIPS Lite simulator
 *
 * @param mips
 */
void init_simulator(MIPSSim *mips, Mode mode) {
  memset(mips, 0, sizeof(MIPSSim));
  mips->mode = mode;
  mips->counts = (InstructionCount){0};
  init_pipeline(&mips->pipeline, !!mode);
  mips->clock = 1;
}

/**
 * @brief Destroy the MIPS Lite simulator
 *
 * @param mips  MIPS simulator
 */
void destroy_simulator(MIPSSim *mips) {
  free(mips);
}

/**
 * @brief Load the program into memory from a file
 *
 * @param mips      MIPS simulator
 * @param filename  File to load
 */
void load_memory(MIPSSim *mips, char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    exit(1);
    return;
  }
  char line[10];
  int i = 0;

  // Read the file line by line and load into memory
  while (fgets(line, sizeof(line), file) && i < MEMORY_SIZE) {
    mips->memory[i].value = strtol(line, NULL, 16);
    i++;
  }
  fclose(file);
  mips->memory_size = i;  // Set the memory size with the number of instructions loaded
  LOG("Memory loaded from file: %s\n", filename);
}

/**
 * @brief Fetch the next instruction from memory (IF stage)
 *
 * @param mips  MIPS simulator
 */
void fetch_stage(MIPSSim *mips) {
  if (!peek_pipeline_stage(&mips->pipeline, IF) && (mips->pc / 4 < mips->memory_size) && !mips->halt) {
    Instruction *instr = (Instruction *)malloc(sizeof(Instruction));
    memset(instr, 0, sizeof(Instruction));
    instr->instruction = mips->memory[mips->pc / 4].value;
    instr->stage = IF;
    fetch_instruction(&mips->pipeline, instr);
    mips->pc += 4;
    // LOG("Instruction fetched: %08x\n", instr->instruction);
  }
}

/**
 * @brief Decode the instruction (ID stage)
 *
 * @param mips  MIPS simulator
 */
void decode_stage(MIPSSim *mips) {
  Instruction *instr = peek_pipeline_stage(&mips->pipeline, mips->pipeline.is_pipelined ? ID : IF);
  if (instr == NULL || instr->stage != ID) {
    return;
  }

  instr->opcode = (instr->instruction >> 26) & INSTR_MASK;

  switch (instr->opcode) {
    case ADD:
    case SUB:
    case MUL:
    case OR:
    case AND:
    case XOR:
      instr->type = R_TYPE;
      break;
    case ADDI:
    case SUBI:
    case MULI:
    case ORI:
    case ANDI:
    case XORI:
      instr->type = I_TYPE_IMM;
      break;
    case LDW:
    case STW:
      instr->type = I_TYPE_MEM;
      break;
    case HALT:
    case BZ:
    case BEQ:
    case JR:
      instr->type = J_TYPE;
      break;
    default:
      fprintf(stderr, "Invalid opcode: %08x\n", instr->instruction);
      exit(1);
      break;
  }

  instr->rs = (instr->instruction >> 21) & INSTR_MASK;
  instr->rt = (instr->instruction >> 16) & INSTR_MASK;

  if (instr->type == R_TYPE) {
    instr->rd = (instr->instruction >> 11) & INSTR_MASK;
  } else {
    instr->imm = (int16_t)(instr->instruction & 0xFFFF);
    instr->alu_out = (int32_t)(mips->pc - 4) + (instr->imm << 2);
  }

  if (mips->mode != NOT_PIPED) check_hazards(mips, instr);

  LOG("DECODED: [Instruction %08x] Type: %d, Opcode: %d, Rs: %d, Rt: %d, Rd: %d, Imm: %d, ALU: %d\n", instr->instruction, instr->type, instr->opcode,
      instr->rs, instr->rt, instr->rd, instr->imm, instr->alu_out);
}

/**
 * @brief Perform the operation based on the opcode in the execute stage
 *
 * @param rs        Source register
 * @param rt        Target register
 * @param opcode    Operation code
 * @return uint32_t Result of the operation
 */
uint32_t perform_operation(uint32_t rs, uint32_t rt, Opcode opcode) {
  // LOG("Performing operation: %d (%d) %d\n", rs, opcode, rt);
  switch (opcode) {
    case ADD:
    case ADDI:
      return rs + rt;
    case SUB:
    case SUBI:
      return rs - rt;
    case MUL:
    case MULI:
      return rs * rt;
    case OR:
    case ORI:
      return rs | rt;
    case AND:
    case ANDI:
      return rs & rt;
    case XOR:
    case XORI:
      return rs ^ rt;
    default:
      return 0;
  }
}

/**
 * @brief Control flow instructions (BZ, BEQ, JR, HALT) handling in the execute stage
 *
 * @param mips  MIPS simulator
 * @param instr Instruction
 * @return true if the branch is taken, false otherwise
 */
bool control_flow(MIPSSim *mips, Instruction *instr, int32_t rs, int32_t rt) {
  switch (instr->opcode) {
    case BZ:  // Branch if zero
      if (rs == 0) {
        mips->pc = instr->alu_out;
        return true;
      }
      break;
    case BEQ:  // Branch if equal
      if (rs == rt) {
        mips->pc = instr->alu_out;
        return true;
      }
      break;
    case JR:  // Jump register
      mips->pc = rs;
      return true;
    case HALT:  // Halt program
      mips->halt = true;
      return true;
    default:
      break;
  }
  return false;
}

/**
 * @brief Execute stage of the pipeline (EX stage)
 *
 * @param mips  MIPS simulator
 */
void execute_stage(MIPSSim *mips) {
  Instruction *instr = peek_pipeline_stage(&mips->pipeline, mips->pipeline.is_pipelined ? EX : IF);
  if (instr == NULL || instr->stage != EX) {
    return;
  }

  if (mips->mode != NOT_PIPED) instr->alu_out = (int32_t)(mips->pc - 8) + (instr->imm << 2);
  int32_t rs = mips->registers[instr->rs].value;
  int32_t rt = mips->registers[instr->rt].value;

  // Forward the register value to either rs or rt if it is forwarded
  if (instr->forward_reg.is_forwarded) {
    if (instr->forward_reg.target == RT)
      rt = instr->forward_reg.reg;
    else
      rs = instr->forward_reg.reg;
  }

  // Perform the operation based on the instruction type
  switch (instr->type) {
    case R_TYPE:  // R-Type instructions (ADD, SUB, MUL, OR, AND, XOR)
      instr->alu_out = perform_operation(rs, rt, instr->opcode);
      break;
    case I_TYPE_IMM:  // I-Type instructions with immediate values (ADDI, SUBI, MULI, ORI, ANDI, XORI)
      instr->alu_out = perform_operation(rs, instr->imm, instr->opcode);
      break;
    case I_TYPE_MEM:  // I-Type memory instructions (LDW, STW)
      instr->alu_out = mips->registers[instr->rs].value + instr->imm;
      break;
    case J_TYPE:  // J-Type instructions (BZ, BEQ, JR, HALT)
      bool branch_taken = control_flow(mips, instr, rs, rt);
      // If branch is taken, flush the pipeline
      if (branch_taken) {
        flush_pipeline(&mips->pipeline, EX);
        mips->clock++;
      }
      break;
    default:
      break;
  }

  // Increment instruction counts
  mips->counts.total++;
  if (instr->opcode >= ADD && instr->opcode <= MULI) {
    mips->counts.arithmetic++;
  } else if (instr->opcode >= OR && instr->opcode <= XORI) {
    mips->counts.logical++;
  } else if (instr->opcode == LDW || instr->opcode == STW) {
    mips->counts.memory++;
  } else {
    mips->counts.control++;
  }
}

void memory_stage(MIPSSim *mips) {
  Instruction *instr = peek_pipeline_stage(&mips->pipeline, mips->pipeline.is_pipelined ? MEM : IF);
  if (instr == NULL || instr->stage != MEM) {
    return;
  }

  // Perform memory operation based on the instruction type
  switch (instr->type) {
    case I_TYPE_MEM:
      if (instr->opcode == LDW) {  // If load word, read from memory and store in MDR
        instr->mdr = mips->memory[instr->alu_out / 4].value;

      } else if (instr->opcode == STW) {  // If store word, write to memory from register
        mips->memory[instr->alu_out / 4].value = mips->registers[instr->rt].value;
        mips->memory[instr->alu_out / 4].modified = true;
      }
      break;
    default:
      break;
  }
}

/**
 * @brief Writeback stage of the pipeline (WB stage)
 *
 * @param mips  MIPS simulator
 */
void writeback_stage(MIPSSim *mips) {
  Instruction *instr = peek_pipeline_stage(&mips->pipeline, mips->pipeline.is_pipelined ? WB : IF);
  if (instr == NULL || instr->stage != WB) {
    return;
  }

  switch (instr->type) {
    case R_TYPE:
      mips->registers[instr->rd].value = instr->alu_out;
      mips->registers[instr->rd].modified = true;
      break;
    case I_TYPE_IMM:
      mips->registers[instr->rt].value = instr->alu_out;
      mips->registers[instr->rt].modified = true;
      break;
    case I_TYPE_MEM:
      if (instr->opcode == LDW) {
        mips->registers[instr->rt].value = instr->mdr;
        mips->registers[instr->rt].modified = true;
      }
      break;
    default:
      break;
  }
}

/**
 * @brief Process the next clock cycle of the simulator
 *
 * @param mips  MIPS simulator
 */
void process(MIPSSim *mips) {
  LOG("-> CLK: %d, PC: %d\n", mips->clock, mips->pc);
  writeback_stage(mips);
  memory_stage(mips);
  execute_stage(mips);
  if (mips->halt) return;
  decode_stage(mips);
  fetch_stage(mips);
  mips->done = advance_pipeline(&mips->pipeline);

  // If not pipelined and the PC is within the memory bounds, keep processing
  if (mips->mode == NOT_PIPED && mips->pc / 4 < mips->memory_size) mips->done = false;
}

void print_memory(MIPSSim *mips) {
  printf("Memory:\n");
  uint8_t k = 0;
  for (int i = 0; i < mips->memory_size; i++) {
    if (mips->memory[i].modified) {
      if (k % 8 == 0 && k != 0) {
        printf("\n");
      }
      printf("[%4d:%d] ", i * 4, mips->memory[i].value);
      k++;
    }
  }
  printf("\n");
}

void print_registers(MIPSSim *mips) {
  printf("Registers:\n");
  uint8_t k = 0;
  for (int i = 0; i < 32; i++) {
    if (mips->registers[i].modified) {
      if (k % 4 == 0 && k != 0) {
        printf("\n");
      }
      printf("[%2d:%4d] ", i, mips->registers[i].value);
      k++;
    }
  }
  printf("\n");
}

/**
 * @brief Correct the PC value if there are instructions in the pipeline that have not been executed
 *
 * @param mips  MIPS simulator
 */
void correct_pc(MIPSSim *mips) {
  if (mips->mode == NOT_PIPED) return;

  for (int i = 0; i < EX; i++) {
    Instruction *instr = peek_pipeline_stage(&mips->pipeline, i);
    if (instr != NULL) {
      mips->pc -= 4;
    }
  }
}

// Helper function to set the forward register
void set_forward_reg(Instruction *instr, int8_t check_reg, int reg) {
  instr->forward_reg = (ForwardReg){.is_forwarded = true, .reg = reg, .target = (instr->rs == check_reg) ? RS : RT};
}

/**
 * @brief Check for RAW hazards and stall the pipeline if necessary
 *
 * @param mips  MIPS simulator
 * @param instr Instruction
 */
void check_hazards(MIPSSim *mips, Instruction *instr) {
  for (int i = EX; i < WB; i++) {
    Instruction *next_instr = peek_pipeline_stage(&mips->pipeline, i);
    if (next_instr == NULL || next_instr->type == J_TYPE) continue;

    int8_t check_reg = (next_instr->type == R_TYPE) ? next_instr->rd : next_instr->rt;

    // Check if the current instruction is using the register that the next instruction will modify
    bool is_hazard = (instr->type == R_TYPE && (instr->rs == check_reg || instr->rt == check_reg)) ||
                     ((instr->type == I_TYPE_IMM || instr->type == I_TYPE_MEM) && instr->rs == check_reg) ||
                     (instr->opcode == BEQ && (instr->rs == check_reg || instr->rt == check_reg)) ||
                     (instr->type == J_TYPE && instr->rs == check_reg);

    if (is_hazard) {
      // If pipeline forwarding is enabled and the next instruction is either R-type or immediate I-type and has reached the EX stage
      if (mips->mode == PIPED_FWD && (next_instr->type == R_TYPE || next_instr->type == I_TYPE_IMM) && next_instr->stage >= EX) {
        set_forward_reg(instr, check_reg, next_instr->alu_out);
        return;

      }
      // If pipeline forwarding is enabled and the next instruction is LDW and has reached MEM stage
      else if (mips->mode == PIPED_FWD && next_instr->opcode == LDW && next_instr->stage >= MEM) {
        set_forward_reg(instr, check_reg, next_instr->mdr);
        return;

      } else {
        stall_pipeline(&mips->pipeline);
        return;
      }
    }
  }
}