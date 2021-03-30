#ifndef __CPU__
#define __CPU__

#include <stdint.h>
#include <stdbool.h>

enum {
// ARCH_X86
  CPU_VENDOR_INTEL,
  CPU_VENDOR_AMD,
// ARCH_ARM
  CPU_VENDOR_ARM,
  CPU_VENDOR_BROADCOM,
  CPU_VENDOR_CAVIUM,
  CPU_VENDOR_NVIDIA,
  CPU_VENDOR_APM,
  CPU_VENDOR_QUALCOMM,
  CPU_VENDOR_HUAWUEI,
  CPU_VENDOR_SAMSUNG,
  CPU_VENDOR_MARVELL,
// OTHERS
  CPU_VENDOR_UNKNOWN,
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
#define CPU_NAME_MAX_LENGTH 64

typedef int32_t VENDOR;

struct frequency {
  int32_t base;
  int32_t max;
};

struct hypervisor {
  bool present;
  char* hv_name;
  VENDOR hv_vendor;
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
  int32_t total_cores;  
  struct cache* cach;
#ifdef ARCH_X86
  uint32_t physical_cores;  
  uint32_t logical_cores;
  uint32_t smt_available; // Number of SMT that is currently enabled
  uint32_t smt_supported; // Number of SMT that CPU supports (equal to smt_available if SMT is enabled)
  uint32_t sockets;
  struct apic* apic;
#endif
};

struct features {
  bool AES; // Must be the first field of features struct!  
#ifdef ARCH_X86
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
  bool SHA;  
#elif ARCH_ARM
  bool NEON;  
  bool SHA1;
  bool SHA2;
  bool CRC32;
#endif  
};

struct cpuInfo {
  VENDOR cpu_vendor;  
  struct uarch* arch;
  struct hypervisor* hv;
  struct frequency* freq;
  struct cache* cach;
  struct topology* topo;
  struct features* feat;
  
#ifdef ARCH_X86
  // CPU name from model
  char* cpu_name;
  //  Max cpuids levels
  uint32_t maxLevels;
  // Max cpuids extended levels
  uint32_t maxExtendedLevels;
  // Topology Extensions (AMD only)
  bool topology_extensions;
#elif ARCH_ARM
  // Main ID register
  uint32_t midr;
#endif

#ifdef ARCH_ARM
  struct system_on_chip* soc;
  // If SoC contains more than one CPU and they
  // are different, the others will be stored in
  // the next_cpu field
  struct cpuInfo* next_cpu;  
  uint8_t num_cpus;
#endif
};

#ifdef ARCH_X86
char* get_str_cpu_name(struct cpuInfo* cpu);
char* get_str_sockets(struct topology* topo);
uint32_t get_nsockets(struct topology* topo);
#endif

VENDOR get_cpu_vendor(struct cpuInfo* cpu);
int64_t get_freq(struct frequency* freq);

char* get_str_aes(struct cpuInfo* cpu);
char* get_str_sha(struct cpuInfo* cpu);
char* get_str_l1i(struct cache* cach);
char* get_str_l1d(struct cache* cach);
char* get_str_l2(struct cache* cach);
char* get_str_l3(struct cache* cach);
char* get_str_freq(struct frequency* freq);

void free_cache_struct(struct cache* cach);
void free_freq_struct(struct frequency* freq);
void free_cpuinfo_struct(struct cpuInfo* cpu);

#endif
