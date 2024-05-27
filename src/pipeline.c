/**
 * @file  pipeline.c
 * @copyright Copyright (c) 2024
 */

#include "pipeline.h"
#include "common.h"

#ifdef DEBUG
static const char *stage_names[] = {"IF", "ID", "EX", "MEM", "WB", "DONE"};
#endif

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
  p->is_stalled = false;
  p->total_stalls = 0;
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
 * @return true if the pipeline is not empty
 */
bool advance_pipeline(Pipeline *p) {
  bool is_empty = true;

  for (int i = NUM_STAGES - 1; i >= 0; i--) {
    Instruction *instr = peek_pipeline_stage(p, i);
    if (instr != NULL) {
      if (instr->stage == WB || instr->stage == DONE) {
        LOG("===> Instruction completed: %08x\n", instr->instruction);
        free(p->stages[i]);
        p->stages[i] = NULL;
        // print_pipeline_state(p);
      } else if (p->is_stalled && instr->stage <= ID) {
        is_empty = false;
        // LOG("===> Stalling %s: %08x\n", stage_names[i], instr->instruction);
      } else {
        is_empty = false;
        // LOG("===> Advancing %s: %08x from %s to %s\n", stage_names[i], instr->instruction, stage_names[i], stage_names[i + 1]);
        instr->stage += 1;
        if (p->is_pipelined) {
          p->stages[i + 1] = instr;
          p->stages[i] = NULL;
        }
      }
    }
  }

  p->is_stalled = false;
  return is_empty;
}

/**
 * @brief Flush the pipeline from a given stage (exclusive) to the beginning. [Only in pipelined mode]
 *
 * @param p     Pipeline
 * @param stage Stage to flush from
 */
void flush_pipeline(Pipeline *p, PipelineStage stage) {
  if (!p->is_pipelined) return;

  for (int i = stage - 1; i >= 0; i--) {
    if (p->stages[i] != NULL) {
      // free(p->stages[i]);
      // p->stages[i] = NULL;
      p->stages[i]->stage = DONE;
    }
  }
}

/**
 * @brief Stall the pipeline in the current cycle
 *
 * @param p Pipeline
 */
void stall_pipeline(Pipeline *p) {
  p->is_stalled = true;
  p->total_stalls++;
}