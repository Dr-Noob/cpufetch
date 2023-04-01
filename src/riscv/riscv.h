#ifndef __RISCV__
#define __RISCV__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info(void);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo);
void print_debug(struct cpuInfo* cpu);

#endif
