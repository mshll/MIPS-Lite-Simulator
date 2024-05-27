/**
 * @file  main.c
 * @brief ECE 486/586 Project: 5-stage MIPS Lite pipeline simulator
 *
 * @author  Team 3: Meshal Almutairi, Abdulaziz Alateeqi, Yousef Alothman, Mohammad Hasan
 * @copyright Copyright (c) 2024
 */

#include "common.h"
#include "mips.h"
#include "pipeline.h"

void process_args(int argc, char* argv[], char** filename, Mode* mode);

int main(int argc, char* argv[]) {
  char* filename = NULL;
  Mode mode;
  process_args(argc, argv, &filename, &mode);

  MIPSSim* mips = malloc(sizeof(MIPSSim));
  init_simulator(mips, mode);
  load_memory(mips, filename);

  while (!mips->done && !mips->halt) {
    process(mips);
  }
  correct_pc(mips);

  printf("======== Simulation complete ========\n");
  printf("Total clock cycles: %d\n", mips->clock);
  printf("Final PC: %d\n", mips->pc);
  printf("Total Stalls: %d\n", mips->pipeline.total_stalls);
  printf("Instruction counts:\n");
  printf("\\ Total: %d\n", mips->counts.total);
  printf("\\ Arithmetic: %d\n", mips->counts.arithmetic);
  printf("\\ Logical: %d\n", mips->counts.logical);
  printf("\\ Memory: %d\n", mips->counts.memory);
  printf("\\ Control: %d\n", mips->counts.control);
  printf("=====================================\n");
  print_registers(mips);
  print_memory(mips);
  if (mips->halt) printf("\n\nPROGRAM HALTED\n");

  destroy_simulator(mips);
  return 0;
}

void process_args(int argc, char* argv[], char** filename, Mode* mode) {
  int opt;
  *mode = -1;

  while ((opt = getopt(argc, argv, "f:m:h")) != -1) {
    switch (opt) {
      case 'f':
        *filename = optarg;
        break;
      case 'm':
        *mode = (Mode)atoi(optarg);
        if (*mode < NOT_PIPED || *mode > PIPED_FWD) {
          fprintf(stderr, "Invalid mode: %d. Mode must be between 0 and 2. Use -h for help\n", *mode);
          exit(EXIT_FAILURE);
        }
        break;
      case 'h':
        fprintf(stderr, "Usage: %s [-f filename] [-m mode]\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -f filename: Load memory image from filename\n");
        fprintf(stderr, "  -m mode: Set the mode (0: Non-pipelined, 1: Pipelined without forwarding, 2: Pipelined with forwarding)\n");
        exit(EXIT_SUCCESS);
      default:
        fprintf(stderr, "Usage: %s [-f filename] [-m mode]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (*mode == -1) {
    fprintf(stderr, "Mode not specified. Please specify a mode using the -m flag. Use -h for help\n");
    exit(EXIT_FAILURE);
  }

  if (*filename == NULL) {
    fprintf(stderr, "Filename not specified. Please specify a filename using the -f flag. Use -h for help\n");
    exit(EXIT_FAILURE);
  }
}