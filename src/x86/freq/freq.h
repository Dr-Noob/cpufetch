#ifndef __FREQ__
#define __FREQ__

#include <stdint.h>
#include "../../common/cpu.h"
#include "../../common/global.h"

#define MEASURE_TIME_SECONDS         5
#define LOOP_ITERS           100000000

int32_t measure_frequency(struct cpuInfo* cpu, int32_t *max_freq_pp_vec);

#endif
