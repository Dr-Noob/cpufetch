#ifndef __CPUID__
#define __CPUID__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info();
struct cache* get_cache_info(struct cpuInfo* cpu);
struct frequency* get_frequency_info(struct cpuInfo* cpu);
struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach);

VENDOR get_cpu_vendor(struct cpuInfo* cpu);
uint32_t get_nsockets(struct topology* topo);
int64_t get_freq(struct frequency* freq);

char* get_str_cpu_name(struct cpuInfo* cpu);
char* get_str_ncores(struct cpuInfo* cpu);
char* get_str_avx(struct cpuInfo* cpu);
char* get_str_sse(struct cpuInfo* cpu);
char* get_str_fma(struct cpuInfo* cpu);
char* get_str_aes(struct cpuInfo* cpu);
char* get_str_sha(struct cpuInfo* cpu);

char* get_str_l1i(struct cache* cach);
char* get_str_l1d(struct cache* cach);
char* get_str_l2(struct cache* cach);
char* get_str_l3(struct cache* cach);

char* get_str_freq(struct frequency* freq);

char* get_str_sockets(struct topology* topo);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket);

char* get_str_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq);

void free_cache_struct(struct cache* cach);
void free_topo_struct(struct topology* topo);
void free_freq_struct(struct frequency* freq);
void free_cpuinfo_struct(struct cpuInfo* cpu);

void debug_cpu_info(struct cpuInfo* cpu);
void debug_cache(struct cache* cach);
void debug_frequency(struct frequency* freq);

#endif
