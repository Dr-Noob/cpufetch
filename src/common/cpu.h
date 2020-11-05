#ifndef __CPU__
#define __CPU__

#include <stdint.h>
#include <stdbool.h>

enum {
  CPU_VENDOR_INTEL,
  CPU_VENDOR_AMD,
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
  struct cache* cach;
#ifdef _ARCH_X86
  struct apic* apic;
#endif
};

#endif
