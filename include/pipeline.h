#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "common.h"

typedef struct {
  Instruction *stages[NUM_STAGES];
  bool is_pipelined;
} Pipeline;

/* Function prototypes */
void init_pipeline(Pipeline *p, bool is_pipelined);
void advance_pipeline(Pipeline *p);
void fetch_instruction(Pipeline *p, Instruction *instr);
Instruction *peek_pipeline_stage(Pipeline *p, PipelineStage stage);
void print_pipeline_state(Pipeline *p);

#endif
