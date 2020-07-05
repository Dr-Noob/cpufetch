#ifndef __CPUID__
#define __CPUID__

#include <stdint.h>

#define VENDOR_EMPTY   0
#define VENDOR_INTEL   1
#define VENDOR_AMD     2
#define VENDOR_INVALID 3

#define UNKNOWN -1

struct cpuInfo;
struct frequency;
struct cache;

struct topology {
  int64_t total_cores;
  uint32_t physical_cores;
  uint32_t logical_cores;
  uint32_t smt_available; // Number of SMT that is currently enabled 
  uint32_t smt_supported; // Number of SMT that CPU supports (equal to smt_available if SMT is enabled)
  uint32_t sockets;  
  struct apic* apic;
};

typedef int32_t VENDOR;

struct cpuInfo* get_cpu_info();
VENDOR get_cpu_vendor(struct cpuInfo* cpu);
uint32_t get_nsockets(struct topology* topo);
int64_t get_freq(struct frequency* freq);
struct cache* get_cache_info(struct cpuInfo* cpu);
struct frequency* get_frequency_info(struct cpuInfo* cpu);
struct topology* get_topology_info(struct cpuInfo* cpu);

char* get_str_cpu_name(struct cpuInfo* cpu);
char* get_str_ncores(struct cpuInfo* cpu);
char* get_str_avx(struct cpuInfo* cpu);
char* get_str_sse(struct cpuInfo* cpu);
char* get_str_fma(struct cpuInfo* cpu);
char* get_str_aes(struct cpuInfo* cpu);
char* get_str_sha(struct cpuInfo* cpu);

char* get_str_l1i(struct cache* cach, struct topology* topo);
char* get_str_l1d(struct cache* cach, struct topology* topo);
char* get_str_l2(struct cache* cach, struct topology* topo);
char* get_str_l3(struct cache* cach, struct topology* topo);

char* get_str_freq(struct frequency* freq);

char* get_str_sockets(struct topology* topo);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket);

char* get_str_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq);

void print_levels(struct cpuInfo* cpu, char* cpu_name);

void free_cpuinfo_struct(struct cpuInfo* cpu);
void free_cache_struct(struct cache* cach);
void free_topo_struct(struct topology* topo);
void free_freq_struct(struct frequency* freq);

void debug_cpu_info(struct cpuInfo* cpu);
void debug_cache(struct cache* cach);
void debug_frequency(struct frequency* freq);

#endif
