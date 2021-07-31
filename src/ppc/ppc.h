#ifndef __POWERPC__
#define __POWERPC__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info();
char* get_str_altivec(struct cpuInfo* cpu);
char* get_str_topology(struct topology* topo, bool dual_socket);
char* get_str_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq);
void print_debug(struct cpuInfo* cpu);

#endif
