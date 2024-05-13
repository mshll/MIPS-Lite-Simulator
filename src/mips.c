/**
 * @file  mips.c
 * @copyright Copyright (c) 2024
 */

#include "mips.h"
#include "common.h"
#include "pipeline.h"

/* helper functions prototypes */
void log_memory(MIPSSim *mips);
void log_registers(MIPSSim *mips);
void adjust_pc(MIPSSim *mips);
void check_hazards(MIPSSim *mips, Instruction *instr);

/**
 * @brief Initialize the MIPS Lite simulator
 *
 * @param mips
 */
void init_simulator(MIPSSim *mips) {
  memset(mips, 0, sizeof(MIPSSim));
  memset(mips->registers, 0, sizeof(mips->registers));
  memset(mips->memory, 0, sizeof(mips->memory));
  mips->memory_size = 0;
  mips->pc = 0;
  mips->clock = 0;
  mips->halt = false;
  mips->counts = (InstructionCount){0};
  init_pipeline(&mips->pipeline, true);  //*** TRUE for pipelined, FALSE for non-pipelined

  log_registers(mips);
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
  log_memory(mips);
}

/**
 * @brief Fetch the next instruction from memory (IF stage)
 *
 * @param mips  MIPS simulator
 */
void fetch_stage(MIPSSim *mips) {
  if (peek_pipeline_stage(&mips->pipeline, IF) == NULL && (mips->pc / 4 < mips->memory_size)) {
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
    instr->alu_out = mips->pc + (instr->imm << 2);
  }

  check_hazards(mips, instr);

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
bool control_flow(MIPSSim *mips, Instruction *instr) {
  switch (instr->opcode) {
    case BZ:  // Branch if zero
      if (mips->registers[instr->rs].value == 0) {
        mips->pc = instr->alu_out - 4;
        return true;
      }
      break;
    case BEQ:  // Branch if equal
      if (mips->registers[instr->rs].value == mips->registers[instr->rt].value) {
        mips->pc = instr->alu_out - 4;
        return true;
      }
      break;
    case JR:  // Jump register
      mips->pc = mips->registers[instr->rs].value;
      return true;
    case HALT:  // Halt program
      mips->halt = true;
      instr->stage = DONE;
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

  uint32_t rs = mips->registers[instr->rs].value;
  uint32_t rt = mips->registers[instr->rt].value;

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
      bool branch_taken = control_flow(mips, instr);
      instr->stage = DONE;
      // If branch is taken, flush the pipeline
      if (branch_taken) {
        flush_pipeline(&mips->pipeline, EX);
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
  print_pipeline_state(&mips->pipeline);
  advance_pipeline(&mips->pipeline);
  mips->clock++;
}

void log_memory(MIPSSim *mips) {
  LOG("Memory:\n");
  uint8_t k = 0;
  for (int i = 0; i < mips->memory_size; i++) {
    if (mips->memory[i].modified) {
      if (k % 8 == 0 && k != 0) {
        LOG("\n");
      }
      LOG("[%4d:%d] ", i * 4, mips->memory[i].value);
      k++;
    }
  }
  LOG("\n");
}

void log_registers(MIPSSim *mips) {
  LOG("Registers:\n");
  uint8_t k = 0;
  for (int i = 0; i < 32; i++) {
    if (mips->registers[i].modified) {
      if (k % 4 == 0 && k != 0) {
        LOG("\n");
      }
      LOG("[%2d:%4d] ", i, mips->registers[i].value);
      k++;
    }
  }
  LOG("\n");
}

void adjust_pc(MIPSSim *mips) {
  for (int i = 0; i <= EX; i++) {
    Instruction *instr = peek_pipeline_stage(&mips->pipeline, i);
    if (instr != NULL) {
      mips->pc -= 4;
      break;
    }
  }
}

/**
 * @brief Check for RAW hazards and stall the pipeline if necessary
 *
 * @param mips  MIPS simulator
 * @param instr Instruction
 */
void check_hazards(MIPSSim *mips, Instruction *instr) {
  for (int i = ID + 1; i < NUM_STAGES; i++) {
    Instruction *next_instr = peek_pipeline_stage(&mips->pipeline, i);
    if (next_instr == NULL) continue;

    int8_t check_reg = -1;
    if (next_instr->type == R_TYPE) {
      check_reg = next_instr->rd;
    } else if (next_instr->type == I_TYPE_IMM || next_instr->type == I_TYPE_MEM) {
      check_reg = next_instr->rt;
    }

    switch (instr->type) {
      case R_TYPE:
        if (instr->rs == check_reg || instr->rt == check_reg) {
          stall_pipeline(&mips->pipeline, i - ID);
          return;
        }
        break;
      case I_TYPE_IMM:
      case I_TYPE_MEM:
        if (instr->rs == check_reg) {
          stall_pipeline(&mips->pipeline, i - ID);
          return;
        }
        break;
      default:
        break;
    }
  }
}