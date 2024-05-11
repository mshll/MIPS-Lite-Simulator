/**
 * @file  mips.c
 * @copyright Copyright (c) 2024
 */

#include "mips.h"
#include "common.h"
#include "pipeline.h"

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

void destroy_simulator(MIPSSim *mips) {
  free(mips);
}

void load_memory(MIPSSim *mips, char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    exit(1);
    return;
  }
  char line[10];
  int i = 0;
  while (fgets(line, sizeof(line), file) && i < MEMORY_SIZE) {
    mips->memory[i].value = strtol(line, NULL, 16);
    i++;
  }
  fclose(file);
  mips->memory_size = i;
  LOG("Memory loaded from file: %s\n", filename);
  log_memory(mips);
}

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

  LOG("DECODED: [Instruction %08x] Type: %d, Opcode: %d, Rs: %d, Rt: %d, Rd: %d, Imm: %d, ALU: %d\n", instr->instruction, instr->type, instr->opcode,
      instr->rs, instr->rt, instr->rd, instr->imm, instr->alu_out);
}

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

bool control_flow(MIPSSim *mips, Instruction *instr) {
  switch (instr->opcode) {
    case BZ:
      if (mips->registers[instr->rs] == 0) {
        mips->pc = instr->alu_out - 4;
        return true;
      }
      break;
    case BEQ:
      if (mips->registers[instr->rs] == mips->registers[instr->rt]) {
        mips->pc = instr->alu_out - 4;
        return true;
      }
      break;
    case JR:
      mips->pc = mips->registers[instr->rs];
      return true;
    case HALT:
      mips->halt = true;
      instr->stage = DONE;
      return true;
    default:
      break;
  }
  return false;
}

void execute_stage(MIPSSim *mips) {
  Instruction *instr = peek_pipeline_stage(&mips->pipeline, mips->pipeline.is_pipelined ? EX : IF);
  if (instr == NULL || instr->stage != EX) {
    return;
  }

  uint32_t rs = mips->registers[instr->rs];
  uint32_t rt = mips->registers[instr->rt];

  switch (instr->type) {
    case R_TYPE:
      instr->alu_out = perform_operation(rs, rt, instr->opcode);
      break;
    case I_TYPE_IMM:
      instr->alu_out = perform_operation(rs, instr->imm, instr->opcode);
      break;
    case I_TYPE_MEM:
      instr->alu_out = mips->registers[instr->rs] + instr->imm;
      break;
    case J_TYPE:
      bool branch_taken = control_flow(mips, instr);
      instr->stage = DONE;
      // if branch taken, point upstream instructions to NOP
      if (branch_taken && mips->pipeline.is_pipelined) {
        for (int i = 0; i < EX; i++) {
          Instruction *upstream = peek_pipeline_stage(&mips->pipeline, i);
          if (upstream != NULL) {
            upstream->stage = DONE;
          }
        }
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

  switch (instr->type) {
    case R_TYPE:
      mips->registers[instr->rd] = instr->alu_out;
      instr->stage = DONE;
      break;
    case I_TYPE_IMM:
      mips->registers[instr->rt] = instr->alu_out;
      instr->stage = DONE;
      break;
    case I_TYPE_MEM:
      if (instr->opcode == LDW) {
        instr->mdr = mips->memory[instr->alu_out / 4].value;
      } else if (instr->opcode == STW) {
        mips->memory[instr->alu_out / 4].value = mips->registers[instr->rt];
        mips->memory[instr->alu_out / 4].modified = true;
        instr->stage = DONE;
      }
      break;
    default:
      break;
  }
}

void writeback_stage(MIPSSim *mips) {
  Instruction *instr = peek_pipeline_stage(&mips->pipeline, mips->pipeline.is_pipelined ? WB : IF);
  if (instr == NULL || instr->stage != WB) {
    return;
  }

  if (instr->type == I_TYPE_MEM) {
    mips->registers[instr->rt] = instr->mdr;
  }
}

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
    if (mips->registers[i] != 0) {
      if (k % 4 == 0 && k != 0) {
        LOG("\n");
      }
      LOG("[%2d:%4d] ", i, mips->registers[i]);
      k++;
    }
  }
  LOG("\n");
}

void adjust_pc(MIPSSim *mips) {
  for (int i = 0; i < EX; i++) {
    Instruction *instr = peek_pipeline_stage(&mips->pipeline, i);
    if (instr != NULL) {
      mips->pc -= 4;
      mips->clock--;
      break;
    }
  }
}