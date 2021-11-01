#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
#else
  #include "../common/udev.h"
  #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "cpuid.h"
#include "cpuid_asm.h"
#include "../common/global.h"
#include "../common/args.h"
#include "apic.h"
#include "uarch.h"
#include "freq.h"

#define CPU_VENDOR_INTEL_STRING "GenuineIntel"
#define CPU_VENDOR_AMD_STRING   "AuthenticAMD"

static const char *hv_vendors_string[] = {
  [HV_VENDOR_KVM]       = "KVMKVMKVM",
  [HV_VENDOR_QEMU]      = "TCGTCGTCGTCG",
  [HV_VENDOR_HYPERV]    = "Microsoft Hv",
  [HV_VENDOR_VMWARE]    = "VMwareVMware",
  [HV_VENDOR_XEN]       = "XenVMMXenVMM",
  [HV_VENDOR_PARALLELS] = "lrpepyh vr",
};

static char *hv_vendors_name[] = {
  [HV_VENDOR_KVM]       = "KVM",
  [HV_VENDOR_QEMU]      = "QEMU",
  [HV_VENDOR_HYPERV]    = "Microsoft Hyper-V",
  [HV_VENDOR_VMWARE]    = "VMware",
  [HV_VENDOR_XEN]       = "Xen",
  [HV_VENDOR_PARALLELS] = "Parallels",
  [HV_VENDOR_INVALID]   = STRING_UNKNOWN
};

#define HYPERVISOR_NAME_MAX_LENGTH 17

#define MASK 0xFF

/*
 * cpuid reference: http://www.sandpile.org/x86/cpuid.htm
 * cpuid amd: https://www.amd.com/system/files/TechDocs/25481.pdf
 */

void get_name_cpuid(char* name, uint32_t reg1, uint32_t reg2, uint32_t reg3) {
  uint32_t c = 0;

  name[c++] = reg1       & MASK;
  name[c++] = (reg1>>8)  & MASK;
  name[c++] = (reg1>>16) & MASK;
  name[c++] = (reg1>>24) & MASK;

  name[c++] = reg2       & MASK;
  name[c++] = (reg2>>8)  & MASK;
  name[c++] = (reg2>>16) & MASK;
  name[c++] = (reg2>>24) & MASK;

  name[c++] = reg3       & MASK;
  name[c++] = (reg3>>8)  & MASK;
  name[c++] = (reg3>>16) & MASK;
  name[c++] = (reg3>>24) & MASK;
}

char* get_str_cpu_name_internal() {
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  uint32_t c = 0;

  char * name = emalloc(sizeof(char) * CPU_NAME_MAX_LENGTH);
  memset(name, 0, CPU_NAME_MAX_LENGTH);

  for(int i=0; i < 3; i++) {
    eax = 0x80000002 + i;
    cpuid(&eax, &ebx, &ecx, &edx);

    name[c++] = eax       & MASK;
    name[c++] = (eax>>8)  & MASK;
    name[c++] = (eax>>16) & MASK;
    name[c++] = (eax>>24) & MASK;
    name[c++] = ebx       & MASK;
    name[c++] = (ebx>>8)  & MASK;
    name[c++] = (ebx>>16) & MASK;
    name[c++] = (ebx>>24) & MASK;
    name[c++] = ecx       & MASK;
    name[c++] = (ecx>>8)  & MASK;
    name[c++] = (ecx>>16) & MASK;
    name[c++] = (ecx>>24) & MASK;
    name[c++] = edx       & MASK;
    name[c++] = (edx>>8)  & MASK;
    name[c++] = (edx>>16) & MASK;
    name[c++] = (edx>>24) & MASK;
  }
  name[c] = '\0';

  //Remove unused characters
  char *str = name;
  char *dest = name;
  // Remove spaces before name
  while (*str != '\0' && *str == ' ')str++;
  // Remove spaces between the name and after it
  while (*str != '\0') {
    while (*str == ' ' && *(str + 1) == ' ') str++;
    *dest++ = *str++;
  }
  *dest = '\0';

  return name;
}

bool abbreviate_intel_cpu_name(char** name) {
  char* old_name = *name;
  char* new_name = ecalloc(strlen(old_name) + 1, sizeof(char));

  char* old_name_ptr = old_name;
  char* new_name_ptr = new_name;
  char* aux_ptr = NULL;

  // 1. Remove "(R)"
  old_name_ptr = strstr(old_name_ptr, "Intel(R)");
  if(old_name_ptr == NULL) return false;
  strcpy(new_name_ptr, "Intel");
  new_name_ptr += strlen("Intel");
  old_name_ptr += strlen("Intel(R)");

  // 2. Remove "(R)" or "(TM)"
  aux_ptr = strstr(old_name_ptr, "(");
  if(aux_ptr == NULL) return false;
  strncpy(new_name_ptr, old_name_ptr, aux_ptr-old_name_ptr);

  new_name_ptr += aux_ptr-old_name_ptr;
  strcpy(new_name_ptr, " ");
  new_name_ptr++;
  old_name_ptr = strstr(aux_ptr, ")");
  if(old_name_ptr == NULL) return false;
  old_name_ptr++;
  while(*old_name_ptr == ' ') old_name_ptr++;

  // 3. Copy the CPU name
  aux_ptr = strstr(old_name_ptr, "@");
  if(aux_ptr == NULL) return false;
  strncpy(new_name_ptr, old_name_ptr, (aux_ptr-1)-old_name_ptr);

  // 4. Remove dummy strings in Intel CPU names
  strremove(new_name, " CPU");
  strremove(new_name, " Dual");
  strremove(new_name, " 0");

  free(old_name);
  *name = new_name;

  return true;
}

struct uarch* get_cpu_uarch(struct cpuInfo* cpu) {
  uint32_t eax = 0x00000001;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;

  cpuid(&eax, &ebx, &ecx, &edx);

  uint32_t stepping = eax & 0xF;
  uint32_t model = (eax >> 4) & 0xF;
  uint32_t emodel = (eax >> 16) & 0xF;
  uint32_t family = (eax >> 8) & 0xF;
  uint32_t efamily = (eax >> 20) & 0xFF;

  return get_uarch_from_cpuid(cpu, eax, efamily, family, emodel, model, (int)stepping);
}

int64_t get_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t max_freq, bool accurate_pp) {
  /*
   * PP = PeakPerformance
   * SP = SinglePrecision
   *
   * PP(SP) =
   * N_CORES                             *
   * FREQUENCY                           *
   * 2(Two vector units)                 *
   * 2(If cpu has fma)                   *
   * 16(If AVX512), 8(If AVX), 4(If SSE) *
   */

  int64_t freq;
#ifdef __linux__
  if(accurate_pp) freq = measure_avx_frequency(cpu);
  else freq = max_freq;
#else
  // Silence compiler warning
  (void)(accurate_pp);
  freq = max_freq;
#endif

  //First, check we have consistent data
  if(freq == UNKNOWN_DATA || topo->logical_cores == UNKNOWN_DATA) {
    return -1;
  }

  struct features* feat = cpu->feat;
  int vpus = get_number_of_vpus(cpu);
  int64_t flops = topo->physical_cores * topo->sockets * (freq*1000000) * vpus;

  if(feat->FMA3 || feat->FMA4)
    flops = flops*2;

  // Ice Lake has AVX512, but it has 1 VPU for AVX512, while
  // it has 2 for AVX2. If this is a Ice Lake CPU, we are computing
  // the peak performance supposing AVX2, not AVX512
  if(feat->AVX512 && vpus_are_AVX512(cpu))
    flops = flops*16;
  else if(feat->AVX || feat->AVX2)
    flops = flops*8;
  else if(feat->SSE)
    flops = flops*4;

  // See https://sites.utexas.edu/jdm4372/2018/01/22/a-peculiar-
  // throughput-limitation-on-intels-xeon-phi-x200-knights-landing/
  if(is_knights_landing(cpu))
    flops = flops * 6 / 7;

  return flops;
}

struct hypervisor* get_hp_info(bool hv_present) {
  struct hypervisor* hv = emalloc(sizeof(struct hypervisor));
  if(!hv_present) {
    hv->present = false;
    return hv;
  }

  hv->present = true;

  uint32_t eax = 0x40000000;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;

  cpuid(&eax, &ebx, &ecx, &edx);

  char name[13];
  memset(name, 0, 13);
  get_name_cpuid(name, ebx, ecx, edx);

  bool found = false;
  uint8_t len = sizeof(hv_vendors_string) / sizeof(hv_vendors_string[0]);

  for(uint8_t v=0; v < len && !found; v++) {
    if(strcmp(hv_vendors_string[v], name) == 0) {
      hv->hv_vendor = v;
      found = true;
    }
  }

  if(!found) {
    hv->hv_vendor = HV_VENDOR_INVALID;
    printWarn("Unknown hypervisor vendor: %s", name);
  }

  hv->hv_name = hv_vendors_name[hv->hv_vendor];

  return hv;
}

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = emalloc(sizeof(struct cpuInfo));
  struct features* feat = emalloc(sizeof(struct features));
  cpu->feat = feat;

  bool *ptr = &(feat->AES);
  for(uint32_t i = 0; i < sizeof(struct features)/sizeof(bool); i++, ptr++) {
    *ptr = false;
  }

  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;

  //Get max cpuid level
  cpuid(&eax, &ebx, &ecx, &edx);
  cpu->maxLevels = eax;

  //Fill vendor
  char name[13];
  memset(name,0,13);
  get_name_cpuid(name, ebx, edx, ecx);

  if(strcmp(CPU_VENDOR_INTEL_STRING,name) == 0)
    cpu->cpu_vendor = CPU_VENDOR_INTEL;
  else if (strcmp(CPU_VENDOR_AMD_STRING,name) == 0)
    cpu->cpu_vendor = CPU_VENDOR_AMD;
  else {
    cpu->cpu_vendor = CPU_VENDOR_INVALID;
    printErr("Unknown CPU vendor: %s", name);
    return NULL;
  }

  //Get max extended level
  eax = 0x80000000;
  ebx = 0;
  ecx = 0;
  edx = 0;
  cpuid(&eax, &ebx, &ecx, &edx);
  cpu->maxExtendedLevels = eax;

  //Fill instructions support
  if (cpu->maxLevels >= 0x00000001){
    eax = 0x00000001;
    cpuid(&eax, &ebx, &ecx, &edx);
    feat->SSE    = (edx & (1U << 25)) != 0;
    feat->SSE2   = (edx & (1U << 26)) != 0;
    feat->SSE3   = (ecx & (1U <<  0)) != 0;

    feat->SSSE3  = (ecx & (1U <<  9)) != 0;
    feat->SSE4_1 = (ecx & (1U << 19)) != 0;
    feat->SSE4_2 = (ecx & (1U << 20)) != 0;

    feat->AES    = (ecx & (1U << 25)) != 0;

    feat->AVX    = (ecx & (1U << 28)) != 0;
    feat->FMA3   = (ecx & (1U << 12)) != 0;

    bool hv_present = (ecx & (1U << 31)) != 0;
    if((cpu->hv = get_hp_info(hv_present)) == NULL)
      return NULL;
  }
  else {
    printWarn("Can't read features information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x00000001, cpu->maxLevels);
  }

  if (cpu->maxLevels >= 0x00000007){
    eax = 0x00000007;
    ecx = 0x00000000;
    cpuid(&eax, &ebx, &ecx, &edx);
    feat->AVX2         = (ebx & (1U <<  5)) != 0;
    feat->SHA          = (ebx & (1U << 29)) != 0;
    feat->AVX512       = (((ebx & (1U << 16)) != 0) ||
                        ((ebx & (1U << 28)) != 0)  ||
                        ((ebx & (1U << 26)) != 0)  ||
                        ((ebx & (1U << 27)) != 0)  ||
                        ((ebx & (1U << 31)) != 0)  ||
                        ((ebx & (1U << 30)) != 0)  ||
                        ((ebx & (1U << 17)) != 0)  ||
                        ((ebx & (1U << 21)) != 0));
  }
  else {
    printWarn("Can't read features information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x00000007, cpu->maxLevels);
  }

  if (cpu->maxExtendedLevels >= 0x80000001){
    eax = 0x80000001;
    cpuid(&eax, &ebx, &ecx, &edx);
    feat->SSE4a = (ecx & (1U <<  6)) != 0;
    feat->FMA4  = (ecx & (1U << 16)) != 0;
  }
  else {
    printWarn("Can't read features information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x80000001, cpu->maxExtendedLevels);
  }

  if (cpu->maxExtendedLevels >= 0x80000004){
    cpu->cpu_name = get_str_cpu_name_internal();
  }
  else {
    cpu->cpu_name = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN) + 1));
    strcpy(cpu->cpu_name, STRING_UNKNOWN);
    printWarn("Can't read cpu name from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x80000004, cpu->maxExtendedLevels);
  }

  cpu->topology_extensions = false;
  if(cpu->cpu_vendor == CPU_VENDOR_AMD && cpu->maxExtendedLevels >= 0x80000001) {
    eax = 0x80000001;
    cpuid(&eax, &ebx, &ecx, &edx);
    cpu->topology_extensions = (ecx >> 22) & 1;
  }

  cpu->arch = get_cpu_uarch(cpu);
  cpu->freq = get_frequency_info(cpu);
  cpu->cach = get_cache_info(cpu);
  cpu->topo = get_topology_info(cpu, cpu->cach);
  cpu->peak_performance = get_peak_performance(cpu, cpu->topo, get_freq(cpu->freq), accurate_pp());

  if(cpu->cach == NULL || cpu->topo == NULL) {
    return NULL;
  }
  return cpu;
}

bool get_cache_topology_amd(struct cpuInfo* cpu, struct topology* topo) {
  if(cpu->maxExtendedLevels >= 0x8000001D && cpu->topology_extensions) {
    uint32_t i, eax, ebx, ecx, edx, num_sharing_cache, cache_type, cache_level;

    i = 0;
    do {
      eax = 0x8000001D;
      ebx = 0;
      ecx = i; // cache id
      edx = 0;

      cpuid(&eax, &ebx, &ecx, &edx);

      cache_type = eax & 0x1F;

      if(cache_type > 0) {
        num_sharing_cache = ((eax >> 14) & 0xFFF) + 1;
        cache_level = (eax >>= 5) & 0x7;

        switch (cache_type) {
          case 1: // Data Cache (We assume this is L1d)
            if(cache_level != 1) {
              printBug("Found data cache at level %d (expected 1)", cache_level);
              return false;
            }
            topo->cach->L1d->num_caches = topo->logical_cores / num_sharing_cache;
            break;

          case 2: // Instruction Cache (We assume this is L1i)
            if(cache_level != 1) {
              printBug("Found instruction cache at level %d (expected 1)", cache_level);
              return false;
            }
            topo->cach->L1i->num_caches = topo->logical_cores / num_sharing_cache;
            break;

          case 3: // Unified Cache (This may be L2 or L3)
            if(cache_level == 2) {
              topo->cach->L2->num_caches = topo->logical_cores / num_sharing_cache;
            }
            else if(cache_level == 3) {
              topo->cach->L3->num_caches = topo->logical_cores / num_sharing_cache;
            }
            else {
              printWarn("Found unknown unified cache at level %d", cache_level);
            }
            break;

          default: // Unknown cache type
            printBug("Unknown cache type %d with level %d found at i=%d", cache_type, cache_level, i);
            return false;
        }
      }

      i++;
    } while (cache_type > 0);
  }
  else {
    printWarn("Can't read topology information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X and topology_extensions=%s). Guessing cache topology", 0x8000001D, cpu->maxExtendedLevels, cpu->topology_extensions ? "true" : "false");
    topo->cach->L1i->num_caches = topo->physical_cores;
    topo->cach->L1d->num_caches = topo->physical_cores;

    if(topo->cach->L3->exists) {
      topo->cach->L2->num_caches = topo->physical_cores;
      topo->cach->L3->num_caches = 1;
    }
    else {
      topo->cach->L2->num_caches = 1;
    }
  }

  return true;
}

// Main reference: https://software.intel.com/content/www/us/en/develop/articles/intel-64-architecture-processor-topology-enumeration.html
// Very interesting resource: https://wiki.osdev.org/Detecting_CPU_Topology_(80x86)
struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach) {
  struct topology* topo = emalloc(sizeof(struct topology));
  init_topology_struct(topo, cach);

  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;

  // Ask the OS the total number of cores it sees
  // If we have one socket, it will be same as the cpuid,
  // but in dual socket it will not!
  // TODO: Replace by apic?
  #ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    topo->total_cores = info.dwNumberOfProcessors;
  #else
    if((topo->total_cores = sysconf(_SC_NPROCESSORS_ONLN)) == -1) {
      printWarn("sysconf(_SC_NPROCESSORS_ONLN): %s", strerror(errno));
      topo->total_cores = topo->logical_cores; // fallback
    }
  #endif

  switch(cpu->cpu_vendor) {
    case CPU_VENDOR_INTEL:
      if (cpu->maxLevels >= 0x00000004) {
        get_topology_from_apic(cpu, topo);
      }
      else {
        printWarn("Can't read topology information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x00000001, cpu->maxLevels);
        topo->physical_cores = 1;
        topo->logical_cores = 1;
        topo->smt_available = 1;
        topo->smt_supported = 1;
      }
      break;
    case CPU_VENDOR_AMD:
      if (cpu->maxExtendedLevels >= 0x80000008) {
        eax = 0x80000008;
        cpuid(&eax, &ebx, &ecx, &edx);
        topo->logical_cores = (ecx & 0xFF) + 1;

        if (cpu->maxExtendedLevels >= 0x8000001E && cpu->topology_extensions) {
          eax = 0x8000001E;
          cpuid(&eax, &ebx, &ecx, &edx);
          topo->smt_supported = ((ebx >> 8) & 0x03) + 1;
        }
        else {
          printWarn("Can't read topology information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X and topology_extensions=%s)", 0x8000001E, cpu->maxExtendedLevels, cpu->topology_extensions ? "true" : "false");
          topo->smt_supported = 1;
        }
      }
      else {
        printWarn("Can't read topology information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x80000008, cpu->maxExtendedLevels);
        topo->physical_cores = 1;
        topo->logical_cores = 1;
        topo->smt_supported = 1;
      }

      if (cpu->maxLevels >= 0x00000001) {
        if(topo->smt_supported > 1)
          topo->smt_available = is_smt_enabled_amd(topo);
        else
          topo->smt_available = 1;
      }
      else {
        printWarn("Can't read topology information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x0000000B, cpu->maxLevels);
        topo->smt_available = 1;
      }
      topo->physical_cores = topo->logical_cores / topo->smt_available;

      if(topo->smt_supported > 1)
        topo->sockets = topo->total_cores / topo->smt_supported / topo->physical_cores; // Idea borrowed from lscpu
      else
        topo->sockets = topo->total_cores / topo->physical_cores;

      get_cache_topology_amd(cpu, topo);

      break;

    default:
      printBug("Cant get topology because VENDOR is empty");
      return NULL;
  }

  return topo;
}

struct cache* get_cache_info_amd_fallback(struct cache* cach) {
  uint32_t eax = 0x80000005;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  cpuid(&eax, &ebx, &ecx, &edx);

  cach->L1d->size = (ecx >> 24) * 1024;
  cach->L1i->size = (edx >> 24) * 1024;

  eax = 0x80000006;
  cpuid(&eax, &ebx, &ecx, &edx);

  cach->L2->size = (ecx >> 16) * 1024;
  cach->L3->size = (edx >> 18) * 512 * 1024;

  cach->L1i->exists = cach->L1i->size > 0;
  cach->L1d->exists = cach->L1d->size > 0;
  cach->L2->exists = cach->L2->size > 0;
  cach->L3->exists = cach->L3->size > 0;

  if(cach->L3->exists)
   cach->max_cache_level = 4;
  else
   cach->max_cache_level = 3;

  return cach;
}

struct cache* get_cache_info_general(struct cache* cach, uint32_t level) {
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  int i=0;
  int32_t cache_type;

  do {
    eax = level; // get cache info
    ebx = 0;
    ecx = i; // cache id
    edx = 0;

    cpuid(&eax, &ebx, &ecx, &edx);

    cache_type = eax & 0x1F;

    // If its 0, we tried fetching a non existing cache
    if (cache_type > 0) {
      int32_t cache_level = (eax >>= 5) & 0x7;
      uint32_t cache_sets = ecx + 1;
      uint32_t cache_coherency_line_size = (ebx & 0xFFF) + 1;
      uint32_t cache_physical_line_partitions = ((ebx >>= 12) & 0x3FF) + 1;
      uint32_t cache_ways_of_associativity = ((ebx >>= 10) & 0x3FF) + 1;

      int32_t cache_total_size = cache_ways_of_associativity * cache_physical_line_partitions * cache_coherency_line_size * cache_sets;
      cach->max_cache_level++;

      switch (cache_type) {
        case 1: // Data Cache (We assume this is L1d)
          if(cache_level != 1) {
            printBug("Found data cache at level %d (expected 1)", cache_level);
            return NULL;
          }
          cach->L1d->size = cache_total_size;
          cach->L1d->exists = true;
          break;

        case 2: // Instruction Cache (We assume this is L1i)
          if(cache_level != 1) {
            printBug("Found instruction cache at level %d (expected 1)", cache_level);
            return NULL;
          }
          cach->L1i->size = cache_total_size;
          cach->L1i->exists = true;
          break;

        case 3: // Unified Cache (This may be L2 or L3)
          if(cache_level == 2) {
            cach->L2->size = cache_total_size;
            cach->L2->exists = true;
          }
          else if(cache_level == 3) {
            cach->L3->size = cache_total_size;
            cach->L3->exists = true;
          }
          else {
            printWarn("Found unknown unified cache at level %d (size is %d bytes)", cache_level, cache_total_size);
            cach->max_cache_level--;
          }
          break;

        default: // Unknown cache type
          printBug("Unknown cache type %d with level %d found at i=%d", cache_type, cache_level, i);
          return NULL;
      }
    }

    i++;
  } while (cache_type > 0);

  return cach;
}

struct cache* get_cache_info(struct cpuInfo* cpu) {
  struct cache* cach = emalloc(sizeof(struct cache));
  init_cache_struct(cach);

  uint32_t level;

  // We use standard 0x00000004 for Intel
  // We use extended 0x8000001D for AMD
  // or 0x80000005/6 for old AMD
  if(cpu->cpu_vendor == CPU_VENDOR_INTEL) {
    level = 0x00000004;
    if(cpu->maxLevels < level) {
      printWarn("Can't read cache information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", level, cpu->maxLevels);
      return NULL;
    }
    else {
      cach = get_cache_info_general(cach, level);
    }
  }
  else {
    level = 0x8000001D;
    if(cpu->maxExtendedLevels < level || !cpu->topology_extensions) {
      printWarn("Can't read cache information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X and topology_extensions=%s)", level, cpu->maxExtendedLevels, cpu->topology_extensions ? "true" : "false");
      level = 0x80000006;
      if(cpu->maxExtendedLevels < level) {
        printWarn("Can't read cache information from cpuid using old method (needed extended level is 0x%.8X, max is 0x%.8X)", level, cpu->maxExtendedLevels);
        return NULL;
      }
      printWarn("Fallback to old method using 0x%.8X and 0x%.8X", level-1, level);
      cach = get_cache_info_amd_fallback(cach);
    }
    else {
      cach = get_cache_info_general(cach, level);
    }
  }

  return cach;
}

struct frequency* get_frequency_info(struct cpuInfo* cpu) {
  struct frequency* freq = emalloc(sizeof(struct frequency));

  if(cpu->maxLevels < 0x00000016) {
    #if defined (_WIN32) || defined (__APPLE__)
      printWarn("Can't read frequency information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x00000016, cpu->maxLevels);
      freq->base = UNKNOWN_DATA;
      freq->max = UNKNOWN_DATA;
    #else
      printWarn("Can't read frequency information from cpuid (needed level is 0x%.8X, max is 0x%.8X). Using udev", 0x00000016, cpu->maxLevels);
      freq->base = UNKNOWN_DATA;
      freq->max = get_max_freq_from_file(0);

      if(freq->max == 0) {
        printWarn("Read max CPU frequency from udev and got 0 MHz");
        freq->max = UNKNOWN_DATA;
      }
    #endif
  }
  else {
    uint32_t eax = 0x00000016;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;

    cpuid(&eax, &ebx, &ecx, &edx);

    freq->base = eax;
    freq->max = ebx;

    if(freq->base == 0) {
      printWarn("Read base CPU frequency from CPUID and got 0 MHz");
      freq->base = UNKNOWN_DATA;
    }
    if(freq->max == 0) {
      printWarn("Read max CPU frequency from CPUID and got 0 MHz");
      #ifdef __linux__
        printWarn("Using udev to detect frequency");
        freq->max = get_max_freq_from_file(0);

        if(freq->max == 0) {
          printWarn("Read max CPU frequency from udev and got 0 MHz");
          freq->max = UNKNOWN_DATA;
        }
      #else
        freq->max = UNKNOWN_DATA;
      #endif
    }
  }

  return freq;
}

// STRING FUNCTIONS
char* get_str_cpu_name_abbreviated(struct cpuInfo* cpu) {
  if(cpu->cpu_vendor == CPU_VENDOR_INTEL) {
    if(!abbreviate_intel_cpu_name(&cpu->cpu_name)) {
      printWarn("Failed to abbreviate CPU name");
    }
  }
  return cpu->cpu_name;
}

char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket) {
  int topo_sockets = dual_socket ? topo->sockets : 1;
  char* string;

  if(topo->logical_cores == UNKNOWN_DATA) {
    string = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN) + 1));
    strcpy(string, STRING_UNKNOWN);
  }
  else if(topo->smt_supported > 1) {
    // 4 for digits, 21 for ' cores (SMT disabled)' which is the longest possible output
    uint32_t max_size = 4+21+1;
    string = emalloc(sizeof(char) * max_size);

    if(topo->smt_available > 1)
      snprintf(string, max_size, "%d cores (%d threads)", topo->physical_cores * topo_sockets, topo->logical_cores * topo_sockets);
    else {
      if(cpu->cpu_vendor == CPU_VENDOR_AMD)
        snprintf(string, max_size, "%d cores (SMT disabled)", topo->physical_cores * topo_sockets);
      else
        snprintf(string, max_size, "%d cores (HT disabled)", topo->physical_cores * topo_sockets);
    }
  }
  else {
    uint32_t max_size = 4+7+1;
    string = emalloc(sizeof(char) * max_size);
    snprintf(string, max_size, "%d cores",topo->physical_cores * topo_sockets);
  }

  return string;
}

char* get_str_avx(struct cpuInfo* cpu) {
  //If all AVX are available, it will use up to 15
  char* string = emalloc(sizeof(char)*17+1);
  if(!cpu->feat->AVX)
    snprintf(string,2+1,"No");
  else if(!cpu->feat->AVX2)
    snprintf(string,3+1,"AVX");
  else if(!cpu->feat->AVX512)
    snprintf(string,8+1,"AVX,AVX2");
  else
    snprintf(string,15+1,"AVX,AVX2,AVX512");

  return string;
}

char* get_str_sse(struct cpuInfo* cpu) {
  uint32_t last = 0;
  uint32_t SSE_sl = 4;
  uint32_t SSE2_sl = 5;
  uint32_t SSE3_sl = 5;
  uint32_t SSSE3_sl = 6;
  uint32_t SSE4a_sl = 6;
  uint32_t SSE4_1_sl = 7;
  uint32_t SSE4_2_sl = 7;
  char* string = emalloc(sizeof(char)*SSE_sl+SSE2_sl+SSE3_sl+SSSE3_sl+SSE4a_sl+SSE4_1_sl+SSE4_2_sl+1);

  if(cpu->feat->SSE) {
      snprintf(string+last,SSE_sl+1,"SSE,");
      last+=SSE_sl;
  }
  if(cpu->feat->SSE2) {
      snprintf(string+last,SSE2_sl+1,"SSE2,");
      last+=SSE2_sl;
  }
  if(cpu->feat->SSE3) {
      snprintf(string+last,SSE3_sl+1,"SSE3,");
      last+=SSE3_sl;
  }
  if(cpu->feat->SSSE3) {
      snprintf(string+last,SSSE3_sl+1,"SSSE3,");
      last+=SSSE3_sl;
  }
  if(cpu->feat->SSE4a) {
      snprintf(string+last,SSE4a_sl+1,"SSE4a,");
      last+=SSE4a_sl;
  }
  if(cpu->feat->SSE4_1) {
      snprintf(string+last,SSE4_1_sl+1,"SSE4.1,");
      last+=SSE4_1_sl;
  }
  if(cpu->feat->SSE4_2) {
      snprintf(string+last,SSE4_2_sl+1,"SSE4.2,");
      last+=SSE4_2_sl;
  }

  //Purge last comma
  string[last-1] = '\0';
  return string;
}

char* get_str_fma(struct cpuInfo* cpu) {
  char* string = emalloc(sizeof(char)*9+1);
  if(!cpu->feat->FMA3)
    snprintf(string,2+1,"No");
  else if(!cpu->feat->FMA4)
    snprintf(string,4+1,"FMA3");
  else
    snprintf(string,9+1,"FMA3,FMA4");

  return string;
}

void print_debug(struct cpuInfo* cpu) {
  uint32_t eax = 0x00000001;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;

  cpuid(&eax, &ebx, &ecx, &edx);

  printf("%s\n", cpu->cpu_name);
  if(cpu->hv->present) {
    printf("- Hypervisor: %s\n", cpu->hv->hv_name);
  }
  printf("- Max standard level: 0x%.8X\n", cpu->maxLevels);
  printf("- Max extended level: 0x%.8X\n", cpu->maxExtendedLevels);
  if(cpu->cpu_vendor == CPU_VENDOR_AMD) {
    printf("- AMD topology extensions: %d\n", cpu->topology_extensions);
  }
  printf("- CPUID dump: 0x%.8X\n", eax);

  free_cpuinfo_struct(cpu);
}

// TODO: Query HV and Xeon Phi levels
void print_raw(struct cpuInfo* cpu) {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  printf("%s\n\n", cpu->cpu_name);
  printf("  CPUID leaf sub   EAX        EBX        ECX        EDX       \n");
  printf("--------------------------------------------------------------\n");

  for(int c=0; c < cpu->topo->total_cores; c++) {
    #ifndef __APPLE__
    if(!bind_to_cpu(c)) {
      printErr("Failed binding to CPU %d", c);
      return;
    }
    #endif

    printf("CPU %d:\n", c);

    for(uint32_t reg=0x00000000; reg <= cpu->maxLevels; reg++) {
      if(reg == 0x00000004) {
        for(uint32_t reg2=0x00000000; reg2 < cpu->cach->max_cache_level; reg2++) {
          eax = reg;
          ebx = 0;
          ecx = reg2;
          edx = 0;

          cpuid(&eax, &ebx, &ecx, &edx);

          printf("  0x%.8X 0x%.2X: 0x%.8X 0x%.8X 0x%.8X 0x%.8X\n", reg, reg2, eax, ebx, ecx, edx);
        }
      }
      else if(reg == 0x0000000B) {
        for(uint32_t reg2=0x00000000; reg2 < cpu->topo->smt_supported; reg2++) {
          eax = reg;
          ebx = 0;
          ecx = reg2;
          edx = 0;

          cpuid(&eax, &ebx, &ecx, &edx);

          printf("  0x%.8X 0x%.2X: 0x%.8X 0x%.8X 0x%.8X 0x%.8X\n", reg, reg2, eax, ebx, ecx, edx);
        }
      }
      else {
        eax = reg;
        ebx = 0;
        ecx = 0;
        edx = 0;

        cpuid(&eax, &ebx, &ecx, &edx);

        printf("  0x%.8X 0x%.2X: 0x%.8X 0x%.8X 0x%.8X 0x%.8X\n", reg, 0x00, eax, ebx, ecx, edx);
      }
    }
    for(uint32_t reg=0x80000000; reg <= cpu->maxExtendedLevels; reg++) {
      if(reg == 0x8000001D) {
        for(uint32_t reg2=0x00000000; reg2 < cpu->cach->max_cache_level; reg2++) {
          eax = reg;
          ebx = 0;
          ecx = reg2;
          edx = 0;

          cpuid(&eax, &ebx, &ecx, &edx);

          printf("  0x%.8X 0x%.2X: 0x%.8X 0x%.8X 0x%.8X 0x%.8X\n", reg, reg2, eax, ebx, ecx, edx);
        }
      }
      else {
        eax = reg;
        ebx = 0;
        ecx = 0;
        edx = 0;

        cpuid(&eax, &ebx, &ecx, &edx);

        printf("  0x%.8X 0x%.2X: 0x%.8X 0x%.8X 0x%.8X 0x%.8X\n", reg, 0x00, eax, ebx, ecx, edx);
      }
    }
  }
}

void free_topo_struct(struct topology* topo) {
  free(topo->apic->cache_select_mask);
  free(topo->apic->cache_id_apic);
  free(topo->apic);
  free(topo);
}
