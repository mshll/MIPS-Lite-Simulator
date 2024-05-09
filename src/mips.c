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
void fetch(MIPS_Init *mips, MIPS_Instruction *instr) {
    if (mips->pc / 4 < MEMORY_SIZE && !mips->halt) {
        instr->instruction = mips->memory[mips->pc / 4];
        instr->stage = ID;  // Move to decode stage
        mips->pc += 4;  // Increment PC
    }
}



void decode(MIPS_Init *mips, MIPS_Instruction *instr) {
    instr->opcode = (instr->instruction >> 26) & 0x3F;
    instr->rs = (instr->instruction >> 21) & 0x1F;
    instr->rt = (instr->instruction >> 16) & 0x1F;
    instr->rd = (instr->instruction >> 11) & 0x1F;
    instr->imm = (short)(instr->instruction & 0xFFFF);  // Sign-extension
}


void executeInstruction(MIPS_Init *mips, MIPS_Instruction *instr) {
    if (mips->halt) return;
    int memValue = 0;
    int writeBackAddr = -1;

    mips->totalInstructions++;  // Increment total instructions counter

    switch (instr->opcode) {
        case 0x00:  // ADD
        case 0x02:  // SUB
        case 0x04:  // MUL
        case 0x06:  // OR
        case 0x08:  // AND
        case 0x0A:  // XOR
            // Execute the operation
            instr->result = performArithmeticOperation(instr->opcode, mips->registers[instr->rs], mips->registers[instr->rt], instr->imm);
            writeBackAddr = instr->rd;
            instr->stage = WB;  // Move directly to Write Back
            break;

        case 0x01:  // ADDI
        case 0x03:  // SUBI
        case 0x05:  // MULI
        case 0x07:  // ORI
        case 0x09:  // ANDI
        case 0x0B:  // XORI
            // Immediate versions also go directly to Write Back
            instr->result = performImmediateOperation(instr->opcode, mips->registers[instr->rs], instr->imm);
            
            writeBackAddr = instr->rt;
            instr->stage = WB;
            break;

        case 0x0C:  // LDW
            memValue = mips->memory[(mips->registers[instr->rs] + instr->imm) / 4];
            instr->result = memValue;
            writeBackAddr = instr->rt;
            instr->stage = MEM;  // Proceed to Memory Access
            break;

        case 0x0D:  // STW
            mips->memory[(mips->registers[instr->rs] + instr->imm) / 4] = mips->registers[instr->rt];
            instr->stage = MEM;  // Proceed to Memory Access
            break;

        case 0x0E:  // BZ
        case 0x0F:  // BEQ
        case 0x10:  // JR
            // Control instructions adjust the PC and bypass MEM and WB stages
            controlFlow(instr, mips);
            instr->stage = DONE;  // Mark as done
            break;

        case 0x11:  // HALT
            mips->halt = 1;  // Set halt flag to stop simulation
            instr->stage = DONE;
            return;

        default:
            printf("Unknown opcode: %u\n", instr->opcode);
            instr->stage = DONE;
    }

    if (writeBackAddr != -1 && instr->stage == WB) {  // If in Write Back stage
        mips->registers[writeBackAddr] = instr->result;
        instr->stage = DONE;  // After Write Back, mark as done
    }

}

int performArithmeticOperation(unsigned int opcode, int rsValue, int rtValue, int immValue) {
    switch (opcode) {
        case 0x00:  // ADD
            return rsValue + rtValue;
        case 0x02:  // SUB
            return rsValue - rtValue;
        case 0x04:  // MUL
            return rsValue * rtValue;
        case 0x06:  // OR
            return rsValue | rtValue;
        case 0x08:  // AND
            return rsValue & rtValue;
        case 0x0A:  // XOR
            return rsValue ^ rtValue;
        default:
            printf("Invalid arithmetic opcode: %u\n", opcode);
            return 0;  // Error case
    }
}
int performImmediateOperation(unsigned int opcode, int rsValue, int immValue) {
    switch (opcode) {
        case 0x01:  // ADDI
            return rsValue + immValue;
        case 0x03:  // SUBI
            return rsValue - immValue;
        case 0x05:  // MULI
            return rsValue * immValue;
        case 0x07:  // ORI
            return rsValue | immValue;
        case 0x09:  // ANDI
            return rsValue & immValue;
        case 0x0B:  // XORI
            return rsValue ^ immValue;
        default:
            printf("Invalid immediate opcode: %u\n", opcode);
            return 0;  // Error case
    }
}

void controlFlow(MIPS_Instruction *instr, MIPS_Init *mips) {
    switch (instr->opcode) {
        case 0x0E:  // BZ (Branch if Zero)
            if (mips->registers[instr->rs] == 0)
                mips->pc += 4 * instr->imm - 4;  // Branching adjusts PC relative to current PC
            break;
        case 0x0F:  // BEQ (Branch if Equal)
            if (mips->registers[instr->rs] == mips->registers[instr->rt])
                mips->pc += 4 * instr->imm - 4;
            break;
        case 0x10:  // JR (Jump Register)
            mips->pc = mips->registers[instr->rs];
            break;
        default:
            printf("Invalid control flow opcode: %u\n", instr->opcode);
    }
}
void memoryAccess(MIPS_Init *mips, MIPS_Instruction *instr) {
    if (instr->opcode == 0x0C) {  // LDW
        instr->result = mips->memory[(mips->registers[instr->rs] + instr->imm) / 4];
    } else if (instr->opcode == 0x0D) {  // STW
        mips->memory[(mips->registers[instr->rs] + instr->imm) / 4] = mips->registers[instr->rt];
    }
}

void writeBack(MIPS_Init *mips, MIPS_Instruction *instr) {
    if (instr->stage == WB) {
        if (instr->opcode == 0x0C || (instr->opcode >= 0x00 && instr->opcode <= 0x0B)) {
            mips->registers[instr->rd] = instr->result;
        }
    }
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
