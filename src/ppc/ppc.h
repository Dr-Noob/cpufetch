#ifndef __POWERPC__
#define __POWERPC__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info();
char* get_str_altivec(struct cpuInfo* cpu);
char* get_str_topology(struct topology* topo, bool dual_socket);
void print_debug(struct cpuInfo* cpu);

#endif
