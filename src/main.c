/**
 * @file  main.c
 * @brief ECE 486/586 Project: 5-stage MIPS Lite pipeline simulator
 *
 * @author  Team 3: Meshal Almutairi, Abdulaziz Alateeqi, Yousef Alothman, Mohammad Hasan
 * @copyright Copyright (c) 2024
 */

#include "common.h"
#include "mips.h"

int main(int argc, char* argv[]) {
  MIPSSim* mips = (MIPSSim*)malloc(sizeof(MIPSSim));
  init_simulator(mips);
  load_memory(mips, "memory_image.txt");

  while (mips->pc / 4 < mips->memory_size && !mips->halt) {
    // LOG("Clock cycle: %d\n", mips->clock);
    process(mips);
  }

  adjust_pc(mips);

  printf("======== Simulation complete ========\n");
  printf("Total clock cycles: %d\n", mips->clock);
  printf("Final PC: %d\n", mips->pc);
  printf("Instruction counts:\n");
  printf("\\ Total: %d\n", mips->counts.total);
  printf("\\ Arithmetic: %d\n", mips->counts.arithmetic);
  printf("\\ Logical: %d\n", mips->counts.logical);
  printf("\\ Memory: %d\n", mips->counts.memory);
  printf("\\ Control: %d\n", mips->counts.control);
  printf("================================\n");

  log_registers(mips);
  log_memory(mips);

  if (mips->halt) {
    printf("\n\nPROGRAM HALTED\n");
  }

  destroy_simulator(mips);
  return 0;
}
