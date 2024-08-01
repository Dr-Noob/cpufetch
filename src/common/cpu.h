#ifndef __CPU__
#define __CPU__

#include <stdint.h>
#include <stdbool.h>

enum {
// ARCH_X86
  CPU_VENDOR_INTEL,
  CPU_VENDOR_AMD,
  CPU_VENDOR_HYGON,
// ARCH_ARM
  CPU_VENDOR_ARM,
  CPU_VENDOR_APPLE,
  CPU_VENDOR_BROADCOM,
  CPU_VENDOR_CAVIUM,
  CPU_VENDOR_NVIDIA,
  CPU_VENDOR_APM,
  CPU_VENDOR_QUALCOMM,
  CPU_VENDOR_HUAWEI,
  CPU_VENDOR_SAMSUNG,
  CPU_VENDOR_MARVELL,
  CPU_VENDOR_PHYTIUM,
// ARCH_RISCV
  CPU_VENDOR_RISCV,
  CPU_VENDOR_SIFIVE,
  CPU_VENDOR_THEAD,
// OTHERS
  CPU_VENDOR_UNKNOWN,
  CPU_VENDOR_INVALID
};

enum {
  HV_VENDOR_KVM,
  HV_VENDOR_QEMU,
  HV_VENDOR_VBOX,
  HV_VENDOR_HYPERV,
  HV_VENDOR_VMWARE,
  HV_VENDOR_XEN,
  HV_VENDOR_PARALLELS,
  HV_VENDOR_PHYP,
  HV_VENDOR_BHYVE,
  HV_VENDOR_APPLEVZ,
  HV_VENDOR_INVALID
};

enum {
  CORE_TYPE_EFFICIENCY,
  CORE_TYPE_PERFORMANCE,
  CORE_TYPE_UNKNOWN
};

#define UNKNOWN_DATA -1
#define CPU_NAME_MAX_LENGTH 64

typedef int32_t VENDOR;

struct frequency {
  int32_t base;
  int32_t max;
  // Indicates if max frequency was measured
  bool measured;
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
#if defined(ARCH_X86) || defined(ARCH_PPC)
  int32_t physical_cores;
  int32_t logical_cores;
  uint32_t sockets;
  uint32_t smt_supported; // Number of SMT that CPU supports (equal to smt_available if SMT is enabled)
#ifdef ARCH_X86
  uint32_t smt_available; // Number of SMT that is currently enabled
  int32_t total_cores_module; // Total cores in the current module (only makes sense in hybrid archs, like ADL)
  struct apic* apic;
#endif
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
#elif ARCH_PPC
  bool altivec;
#elif ARCH_ARM
  bool NEON;  
  bool SHA1;
  bool SHA2;
  bool CRC32;
  bool SVE;
  bool SVE2;
  uint32_t cntb;
#endif  
};

struct extensions {
  char* str;
  uint64_t mask;
};

struct cpuInfo {
  VENDOR cpu_vendor;
  struct uarch* arch;
  struct hypervisor* hv;
  struct frequency* freq;
  struct cache* cach;
  struct topology* topo;
  int64_t peak_performance;

  // Similar but not exactly equal
  // to struct features
#ifdef ARCH_RISCV
  struct extensions* ext;
#else
  struct features* feat;
#endif

#if defined(ARCH_X86) || defined(ARCH_PPC)
  // CPU name from model
  char* cpu_name;
#endif

#ifdef ARCH_X86
  //  Max cpuids levels
  uint32_t maxLevels;
  // Max cpuids extended levels
  uint32_t maxExtendedLevels;
  // Topology Extensions (AMD only)
  bool topology_extensions;
  // Hybrid Flag (Intel only)
  bool hybrid_flag;
  // Core Type (P/E)
  uint32_t core_type;
#elif ARCH_PPC
  uint32_t pvr;
#elif ARCH_ARM
  // Main ID register
  uint32_t midr;
#endif

#if defined(ARCH_ARM) || defined(ARCH_RISCV)
  struct system_on_chip* soc;
#endif

#if defined(ARCH_X86) || defined(ARCH_ARM)
  // If SoC contains more than one CPU and they
  // are different, the others will be stored in
  // the next_cpu field
  struct cpuInfo* next_cpu;
  uint8_t num_cpus;
#ifdef ARCH_X86
  // The index of the first core in the module
  uint32_t first_core_id;
#endif
#endif
};

#if defined(ARCH_X86) || defined(ARCH_PPC)
char* get_str_cpu_name(struct cpuInfo* cpu, bool fcpuname);
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
char* get_str_peak_performance(int64_t flops);

void init_topology_struct(struct topology* topo, struct cache* cach);
void init_cache_struct(struct cache* cach);

void free_cache_struct(struct cache* cach);
void free_freq_struct(struct frequency* freq);
void free_cpuinfo_struct(struct cpuInfo* cpu);

#endif
