#ifndef __SOC_RISCV__
#define __SOC_RISCV__

#include "../common/soc.h"
#include "../common/cpu.h"
#include <stdint.h>

struct system_on_chip* get_soc(struct cpuInfo* cpu);

#endif
