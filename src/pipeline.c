/**
 * @file  pipeline.c
 * @copyright Copyright (c) 2024
 */

#include "pipeline.h"
#include "common.h"

void init_pipeline(Pipeline *p) {
  for (int i = 0; i < NUM_STAGES; i++) {
    p->stages[i] = NULL;
  }
}

void fetch_instruction(Pipeline *p, Instruction *instr) {
  if (p->stages[IF] == NULL) {
    p->stages[IF] = instr;
  }
}

Instruction *peek_pipeline_stage(Pipeline *p, PipelineStage stage) {
  return p->stages[stage];
}

void print_pipeline_state(Pipeline *p) {
  const char *stage_names[] = {"IF", "ID", "EX", "MEM", "WB", "DONE"};

  for (int i = 0; i < NUM_STAGES; i++) {
    Instruction *instr = peek_pipeline_stage(p, i);
    if (instr != NULL) {
      LOG("%s: %08x \t", stage_names[i], instr->instruction);
    } else {
      LOG("%s: -------- \t", stage_names[i]);
    }
  }
  LOG("\n");
}

void advance_pipeline(Pipeline *p) {
  const char *stage_names[] = {"IF", "ID", "EX", "MEM", "WB", "DONE"};
  for (int i = NUM_STAGES - 1; i >= 0; i--) {
    Instruction *instr = peek_pipeline_stage(p, i);
    if (instr != NULL) {
      if (i == WB || instr->stage == DONE) {
        LOG("===> Instruction completed: %08x\n", instr->instruction);
        free(p->stages[i]);
        p->stages[i] = NULL;
        // print_pipeline_state(p);
      } else {
        // LOG("===> Advancing %s: %08x from %s to %s\n", stage_names[i], instr->instruction, stage_names[i], stage_names[i + 1]);
        instr->stage = i + 1;
        p->stages[i + 1] = instr;
        p->stages[i] = NULL;
      }
    }
  }
}