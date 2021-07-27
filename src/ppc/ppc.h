#ifndef __POWERPC__
#define __POWERPC__

#include "../common/cpu.h"

struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach);
struct cpuInfo* get_cpu_info();
void print_debug(struct cpuInfo* cpu);

#endif
