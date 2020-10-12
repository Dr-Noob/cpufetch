#ifndef __CPUID__
#define __CPUID__

#include <stdint.h>

enum {
  CPU_VENDOR_INTEL,
  CPU_VENDOR_AMD,
  CPU_VENDOR_INVALID
};

enum {
  HV_VENDOR_KVM,
  HV_VENDOR_QEMU,
  HV_VENDOR_HYPERV,
  HV_VENDOR_VMWARE,
  HV_VENDOR_XEN,
  HV_VENDOR_PARALLELS,
  HV_VENDOR_INVALID
};

#define UNKNOWN_FREQ -1

typedef int32_t VENDOR;

struct frequency;

struct hypervisor {
  bool present;
  char* hv_name;
  VENDOR hv_vendor;
};

struct cpuInfo {
  bool AVX;
  bool AVX2;
  bool AVX512;
  bool SSE;
  bool SSE2;
  bool SSE3;
  bool SSSE3;
  bool SSE4a;
  bool SSE4_1;
  bool SSE4_2;
  bool FMA3;
  bool FMA4;
  bool AES;
  bool SHA;

  VENDOR cpu_vendor;
  
  char* cpu_name;
  //  Max cpuids levels
  uint32_t maxLevels;
  // Max cpuids extended levels
  uint32_t maxExtendedLevels;
  
  struct uarch* arch;
  struct hypervisor* hv;
};

struct cach {
  int32_t size;
  uint8_t num_caches;
  bool exists;
  // plenty of more properties to include in the future...
};

struct cache {
  struct cach* L1i;
  struct cach* L1d;
  struct cach* L2;
  struct cach* L3;
  struct cach** cach_arr;
  
  uint8_t max_cache_level;
};

struct topology {
  int64_t total_cores;
  uint32_t physical_cores;
  uint32_t logical_cores;
  uint32_t smt_available; // Number of SMT that is currently enabled 
  uint32_t smt_supported; // Number of SMT that CPU supports (equal to smt_available if SMT is enabled)
  uint32_t sockets;  
  struct apic* apic;
  struct cache* cach;
};

struct cpuInfo* get_cpu_info();
VENDOR get_cpu_vendor(struct cpuInfo* cpu);
uint32_t get_nsockets(struct topology* topo);
int64_t get_freq(struct frequency* freq);
struct cache* get_cache_info(struct cpuInfo* cpu);
struct frequency* get_frequency_info(struct cpuInfo* cpu);
struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach);

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

void print_levels(struct cpuInfo* cpu);

void free_cache_struct(struct cache* cach);
void free_topo_struct(struct topology* topo);
void free_freq_struct(struct frequency* freq);
void free_cpuinfo_struct(struct cpuInfo* cpu);

void debug_cpu_info(struct cpuInfo* cpu);
void debug_cache(struct cache* cach);
void debug_frequency(struct frequency* freq);

#endif
