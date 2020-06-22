#ifndef __01h__
#define __01h__

#include <stdint.h>

#define VENDOR_EMPTY   0
#define VENDOR_INTEL   1
#define VENDOR_AMD     2
#define VENDOR_INVALID 3

#define UNKNOWN -1

struct cpuInfo;
struct frequency;
struct cache;
struct topology;

typedef int32_t VENDOR;

struct cpuInfo* get_cpu_info();
VENDOR get_cpu_vendor(struct cpuInfo* cpu);
int64_t get_freq(struct frequency* freq);
struct cache* get_cache_info(struct cpuInfo* cpu);
struct frequency* get_frequency_info(struct cpuInfo* cpu);
struct topology* get_topology_info(struct cpuInfo* cpu);

char* get_str_ncores(struct cpuInfo* cpu);
char* get_str_avx(struct cpuInfo* cpu);
char* get_str_sse(struct cpuInfo* cpu);
char* get_str_fma(struct cpuInfo* cpu);
char* get_str_aes(struct cpuInfo* cpu);
char* get_str_sha(struct cpuInfo* cpu);

char* get_str_l1(struct cache* cach);
char* get_str_l2(struct cache* cach);
char* get_str_l3(struct cache* cach);

char* get_str_freq(struct frequency* freq);

char* get_str_topology(struct topology* topo);

char* get_str_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq);

void free_cpuinfo_struct(struct cpuInfo* cpu);
void free_cache_struct(struct cache* cach);
void free_freq_struct(struct frequency* freq);

void debug_cpu_info(struct cpuInfo* cpu);
void debug_cache(struct cache* cach);
void debug_frequency(struct frequency* freq);

#endif
