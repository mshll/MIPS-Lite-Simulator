#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "common.h"

typedef struct {
  Instruction *stages[NUM_STAGES];
  bool is_pipelined;
  uint32_t stall_cycles;
  uint32_t total_stalls;
} Pipeline;

/* Function prototypes */
void init_pipeline(Pipeline *p, bool is_pipelined);
bool advance_pipeline(Pipeline *p);
void fetch_instruction(Pipeline *p, Instruction *instr);
Instruction *peek_pipeline_stage(Pipeline *p, PipelineStage stage);
void print_pipeline_state(Pipeline *p);
void flush_pipeline(Pipeline *p, PipelineStage stage);
void stall_pipeline(Pipeline *p, int n);

#endif
