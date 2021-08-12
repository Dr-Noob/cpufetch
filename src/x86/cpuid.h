#ifndef __CPUID__
#define __CPUID__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info();
struct cache* get_cache_info(struct cpuInfo* cpu);
struct frequency* get_frequency_info(struct cpuInfo* cpu);
struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach);

char* get_str_avx(struct cpuInfo* cpu);
char* get_str_sse(struct cpuInfo* cpu);
char* get_str_fma(struct cpuInfo* cpu);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket);
char* get_str_cpu_name_abbreviated(struct cpuInfo* cpu);

void print_debug(struct cpuInfo* cpu);
void print_raw(struct cpuInfo* cpu);

void free_topo_struct(struct topology* topo);

#endif
