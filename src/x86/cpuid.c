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
#include "apic.h"
#include "uarch.h"

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
  [HV_VENDOR_INVALID]   = "Unknown"
};

#define STRING_UNKNOWN    "Unknown"

#define HYPERVISOR_NAME_MAX_LENGTH 17

#define MASK 0xFF

/*
 * cpuid reference: http://www.sandpile.org/x86/cpuid.htm
 * cpuid amd: https://www.amd.com/system/files/TechDocs/25481.pdf
 */

void init_topology_struct(struct topology* topo, struct cache* cach) {
  topo->total_cores = 0;
  topo->physical_cores = 0;
  topo->logical_cores = 0;
  topo->smt_available = 0;
  topo->smt_supported = 0;
  topo->sockets = 0;
  topo->apic = malloc(sizeof(struct apic));
  topo->cach = cach;
}

void init_cache_struct(struct cache* cach) {
  cach->L1i = malloc(sizeof(struct cach));
  cach->L1d = malloc(sizeof(struct cach));
  cach->L2 = malloc(sizeof(struct cach));
  cach->L3 = malloc(sizeof(struct cach));
  
  cach->cach_arr = malloc(sizeof(struct cach*) * 4);
  cach->cach_arr[0] = cach->L1i;
  cach->cach_arr[1] = cach->L1d;
  cach->cach_arr[2] = cach->L2;
  cach->cach_arr[3] = cach->L3;
  
  cach->max_cache_level = 0;
  cach->L1i->exists = false;
  cach->L1d->exists = false;
  cach->L2->exists = false;
  cach->L3->exists = false;
}

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
  
  char * name = malloc(sizeof(char) * CPU_NAME_MAX_LENGTH);
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
  
  return get_uarch_from_cpuid(cpu, efamily, family, emodel, model, (int)stepping);
}

struct hypervisor* get_hp_info(bool hv_present) {
  struct hypervisor* hv = malloc(sizeof(struct hypervisor));
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
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  struct features* feat = malloc(sizeof(struct features));
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
    feat->SSE    = (edx & ((int)1 << 25)) != 0;
    feat->SSE2   = (edx & ((int)1 << 26)) != 0;
    feat->SSE3   = (ecx & ((int)1 <<  0)) != 0;

    feat->SSSE3  = (ecx & ((int)1 <<  9)) != 0;
    feat->SSE4_1 = (ecx & ((int)1 << 19)) != 0;
    feat->SSE4_2 = (ecx & ((int)1 << 20)) != 0;

    feat->AES    = (ecx & ((int)1 << 25)) != 0;

    feat->AVX    = (ecx & ((int)1 << 28)) != 0;
    feat->FMA3   = (ecx & ((int)1 << 12)) != 0;
    
    bool hv_present = (ecx & ((int)1 << 31)) != 0;    
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
    feat->AVX2         = (ebx & ((int)1 <<  5)) != 0;
    feat->SHA          = (ebx & ((int)1 << 29)) != 0;
    feat->AVX512       = (((ebx & ((int)1 << 16)) != 0) ||
                        ((ebx & ((int)1 << 28)) != 0)  ||
                        ((ebx & ((int)1 << 26)) != 0)  ||
                        ((ebx & ((int)1 << 27)) != 0)  ||
                        ((ebx & ((int)1 << 31)) != 0)  ||
                        ((ebx & ((int)1 << 30)) != 0)  ||
                        ((ebx & ((int)1 << 17)) != 0)  ||
                        ((ebx & ((int)1 << 21)) != 0));
  }
  else {
    printWarn("Can't read features information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x00000007, cpu->maxLevels);    
  }
  
  if (cpu->maxExtendedLevels >= 0x80000001){
    eax = 0x80000001;
    cpuid(&eax, &ebx, &ecx, &edx);
    feat->SSE4a = (ecx & ((int)1 <<  6)) != 0;
    feat->FMA4  = (ecx & ((int)1 << 16)) != 0;
  }
  else {
    printWarn("Can't read features information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x80000001, cpu->maxExtendedLevels);        
  }
  
  if (cpu->maxExtendedLevels >= 0x80000004){
    cpu->cpu_name = get_str_cpu_name_internal();
  }
  else {    
    cpu->cpu_name = malloc(sizeof(char)*8);
    sprintf(cpu->cpu_name,"Unknown");
    printWarn("Can't read cpu name from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x80000004, cpu->maxExtendedLevels);        
  }
  
  cpu->arch = get_cpu_uarch(cpu);
  cpu->freq = get_frequency_info(cpu);
  cpu->cach = get_cache_info(cpu);
  cpu->topo = get_topology_info(cpu, cpu->cach);
  
  if(cpu->cach == NULL || cpu->topo == NULL) {
    return NULL;
  }
  return cpu;
}

bool get_cache_topology_amd(struct cpuInfo* cpu, struct topology* topo) {    
  if(cpu->maxExtendedLevels >= 0x8000001D) {  
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
    printWarn("Can't read topology information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X). Guessing cache sizes", 0x8000001D, cpu->maxExtendedLevels); 
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
  struct topology* topo = malloc(sizeof(struct topology));  
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
      perror("sysconf");
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
 
        if (cpu->maxExtendedLevels >= 0x8000001E) {
          eax = 0x8000001E;  
          cpuid(&eax, &ebx, &ecx, &edx);
          topo->smt_supported = ((ebx >> 8) & 0x03) + 1;          
        }
        else {
          printWarn("Can't read topology information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x8000001E, cpu->maxExtendedLevels); 
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
  struct cache* cach = malloc(sizeof(struct cache));
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
    if(cpu->maxExtendedLevels < level) {
      printWarn("Can't read cache information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", level, cpu->maxExtendedLevels);
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

  // Sanity checks. If we read values greater than this, they can't be valid ones
  // The values were chosen by me
  if(cach->L1i->size > 64 * 1024) {
    printBug("Invalid L1i size: %dKB", cach->L1i->size/1024);
  }
  if(cach->L1d->size > 64 * 1024) {
    printBug("Invalid L1d size: %dKB", cach->L1d->size/1024);
  }
  if(cach->L2->exists) {
    if(cach->L3->exists && cach->L2->size > 2 * 1048576) {
      printBug("Invalid L2 size: %dMB", cach->L2->size/(1048576));
    }
    else if(cach->L2->size > 100 * 1048576) {
      printBug("Invalid L2 size: %dMB", cach->L2->size/(1048576));
    }
  }
  if(cach->L3->exists && cach->L3->size > 100 * 1048576) {
    printBug("Invalid L3 size: %dMB", cach->L3->size/(1048576));
  }
  if(!cach->L2->exists) {
    printBug("Could not find L2 cache");
  }

  return cach;
}

struct frequency* get_frequency_info(struct cpuInfo* cpu) {
  struct frequency* freq = malloc(sizeof(struct frequency));
  
  if(cpu->maxLevels < 0x00000016) {
    #if defined (_WIN32) || defined (__APPLE__)
      printWarn("Can't read frequency information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x00000016, cpu->maxLevels);
      freq->base = UNKNOWN_FREQ;
      freq->max = UNKNOWN_FREQ;
    #else
      printWarn("Can't read frequency information from cpuid (needed level is 0x%.8X, max is 0x%.8X). Using udev", 0x00000016, cpu->maxLevels);
      freq->base = UNKNOWN_FREQ;
      freq->max = get_max_freq_from_file(0, cpu->hv->present);

      if(freq->max == 0) {
        if(cpu->hv->present) {
          printWarn("Read max CPU frequency and got 0 MHz");
        }
        else {
          printBug("Read max CPU frequency and got 0 MHz");
        }
        freq->max = UNKNOWN_FREQ;
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
      if(cpu->hv->present) {
        printWarn("Read base CPU frequency and got 0 MHz");
      }
      else {
        printBug("Read base CPU frequency and got 0 MHz");    
      }
      freq->base = UNKNOWN_FREQ;
    }
    if(freq->max == 0) {
      if(cpu->hv->present) {
        printWarn("Read max CPU frequency and got 0 MHz");
      }
      else {
        printBug("Read max CPU frequency and got 0 MHz");    
      }
      freq->max = UNKNOWN_FREQ;
    }
  }
  
  return freq;
}

/*** STRING FUNCTIONS ***/

char* get_str_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq) {
  /***
  PP = PeakPerformance
  SP = SinglePrecision

  PP(SP) =
  N_CORES                             *
  FREQUENCY                           *
  2(Two vector units)                 *
  2(If cpu has fma)                   *
  16(If AVX512), 8(If AVX), 4(If SSE) *

  ***/

  //7 for GFLOP/s and 6 for digits,eg 412.14
  uint32_t size = 7+6+1+1;
  assert(strlen(STRING_UNKNOWN)+1 <= size);
  char* string = malloc(sizeof(char)*size);

  //First check we have consistent data
  if(freq == UNKNOWN_FREQ) {
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
    return string;
  }

  struct features* feat = cpu->feat;
  double flops = topo->physical_cores * topo->sockets * (freq*1000000);
  int vpus = get_number_of_vpus(cpu);
  
  flops = flops * vpus; 

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

  if(flops >= (double)1000000000000.0)
    snprintf(string,size,"%.2f TFLOP/s",flops/1000000000000);
  else if(flops >= 1000000000.0)
    snprintf(string,size,"%.2f GFLOP/s",flops/1000000000);
  else
    snprintf(string,size,"%.2f MFLOP/s",flops/1000000);
  
  return string;
}

// TODO: Refactoring
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket) {
  char* string;
  if(topo->smt_supported > 1) {
    //3 for digits, 21 for ' cores (SMT disabled)' which is the longest possible output
    uint32_t size = 3+21+1;
    string = malloc(sizeof(char)*size);
    if(dual_socket) {
      if(topo->smt_available > 1)
        snprintf(string, size, "%d cores (%d threads)",topo->physical_cores * topo->sockets, topo->logical_cores * topo->sockets);
      else {
        if(cpu->cpu_vendor == CPU_VENDOR_AMD)
          snprintf(string, size, "%d cores (SMT disabled)",topo->physical_cores * topo->sockets);
        else
          snprintf(string, size, "%d cores (HT disabled)",topo->physical_cores * topo->sockets);
      }
    }
    else {
      if(topo->smt_available > 1)
        snprintf(string, size, "%d cores (%d threads)",topo->physical_cores,topo->logical_cores);
      else {
        if(cpu->cpu_vendor == CPU_VENDOR_AMD)
          snprintf(string, size, "%d cores (SMT disabled)",topo->physical_cores);
        else
          snprintf(string, size, "%d cores (HT disabled)",topo->physical_cores);
      }
    }
  }
  else {
    uint32_t size = 3+7+1;
    string = malloc(sizeof(char)*size);
    if(dual_socket)
      snprintf(string, size, "%d cores",topo->physical_cores * topo->sockets);
    else
      snprintf(string, size, "%d cores",topo->physical_cores);
  }
  return string;
}

char* get_str_avx(struct cpuInfo* cpu) {
  //If all AVX are available, it will use up to 15
  char* string = malloc(sizeof(char)*17+1);
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
  char* string = malloc(sizeof(char)*SSE_sl+SSE2_sl+SSE3_sl+SSSE3_sl+SSE4a_sl+SSE4_1_sl+SSE4_2_sl+1);

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
  char* string = malloc(sizeof(char)*9+1);
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
  if(cpu->hv->present) printf("- Hypervisor: %s\n", cpu->hv->hv_name);
  printf("- Max standard level: 0x%.8X\n", cpu->maxLevels);
  printf("- Max extended level: 0x%.8X\n", cpu->maxExtendedLevels);
  printf("- CPUID dump: 0x%.8X\n", eax);

  free_cpuinfo_struct(cpu);
}

void free_topo_struct(struct topology* topo) {
  free(topo->apic->cache_select_mask);
  free(topo->apic->cache_id_apic);
  free(topo->apic);
  free(topo);
}
