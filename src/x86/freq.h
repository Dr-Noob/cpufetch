#ifndef __FREQ__
#define __FREQ__

#include <stdint.h>
#include "../common/cpu.h"

int64_t measure_avx_frequency(struct cpuInfo* cpu);

#endif
