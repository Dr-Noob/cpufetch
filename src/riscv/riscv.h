#ifndef __RISCV__
#define __RISCV__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info(void);
void print_debug(struct cpuInfo* cpu);

#endif
