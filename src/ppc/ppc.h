#ifndef __CPUFETCH_POWERPC__
#define __CPUFETCH_POWERPC__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info(void);
char* get_str_altivec(struct cpuInfo* cpu);
char* get_str_topology(struct topology* topo, bool dual_socket);
void print_debug(struct cpuInfo* cpu);

#endif
