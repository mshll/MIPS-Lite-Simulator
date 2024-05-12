/**
 * @file  pipeline.c
 * @copyright Copyright (c) 2024
 */

#include "pipeline.h"
#include "common.h"

/**
 * @brief Initialize the pipeline
 *
 * @param p             Pipeline
 * @param is_pipelined  Use pipelined or non-pipelined mode
 */
void init_pipeline(Pipeline *p, bool is_pipelined) {
  for (int i = 0; i < NUM_STAGES; i++) {
    p->stages[i] = NULL;
  }
  p->is_pipelined = is_pipelined;
}

/**
 * @brief Fetch an instruction into the pipeline IF stage
 *
 * @param p     Pipeline
 * @param instr Instruction to fetch
 */
void fetch_instruction(Pipeline *p, Instruction *instr) {
  if (p->stages[IF] == NULL) {
    p->stages[IF] = instr;
  }
}

/**
 * @brief Peek at the instruction in a pipeline stage
 *
 * @param p     Pipeline
 * @param stage Pipeline stage
 * @return Instruction in the stage
 */
Instruction *peek_pipeline_stage(Pipeline *p, PipelineStage stage) {
  return p->stages[stage];
}

/**
 * @brief Print the current state of the pipeline
 *
 * @param p Pipeline
 */
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

/**
 * @brief Advance the pipeline by moving instructions to the next stage or freeing them
 *
 * @param p Pipeline
 */
void advance_pipeline(Pipeline *p) {
  for (int i = NUM_STAGES - 1; i >= 0; i--) {
    Instruction *instr = peek_pipeline_stage(p, i);
    if (instr != NULL) {
      if (instr->stage == WB || instr->stage == DONE) {
        LOG("===> Instruction completed: %08x\n", instr->instruction);
        free(p->stages[i]);
        p->stages[i] = NULL;
        // print_pipeline_state(p);
      } else {
        // LOG("===> Advancing %s: %08x from %s to %s\n", stage_names[i], instr->instruction, stage_names[i], stage_names[i + 1]);
        instr->stage += 1;
        if (p->is_pipelined) {
          p->stages[i + 1] = instr;
          p->stages[i] = NULL;
        }
      }
    }
  }
}

/**
 * @brief Flush the pipeline from a given stage to the beginning
 *
 * @param p     Pipeline
 * @param stage Stage to flush from
 */
void flush_pipeline(Pipeline *p, PipelineStage stage) {
  for (int i = stage; i >= 0; i--) {
    if (p->stages[i] != NULL) {
      free(p->stages[i]);
      p->stages[i] = NULL;
    }
  }
}