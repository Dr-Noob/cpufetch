#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include "udev.h"
#endif

#include "cpuid.h"
#include "cpuid_asm.h"
#include "global.h"

#define VENDOR_INTEL_STRING "GenuineIntel"
#define VENDOR_AMD_STRING   "AuthenticAMD"

#define STRING_YES        "Yes"
#define STRING_NO         "No"
#define STRING_UNKNOWN    "Unknown"
#define STRING_NONE       "None"
#define STRING_MEGAHERZ   "MHz"
#define STRING_GIGAHERZ   "GHz"
#define STRING_KILOBYTES  "KB"
#define STRING_MEGABYTES  "MB"

#define CPU_NAME_MAX_LENGTH 64

#define MASK 0xFF

/*
 * cpuid reference: http://www.sandpile.org/x86/cpuid.htm
 * cpuid amd: https://www.amd.com/system/files/TechDocs/25481.pdf
 */

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
};

struct cache {
  int32_t L1i;
  int32_t L1d;
  int32_t L2;
  int32_t L3;
};

struct frequency {
  int64_t base;
  int64_t max;
};

struct topology {
  int64_t total_cores;
  uint32_t physical_cores;
  uint32_t logical_cores;
  uint32_t smt;  
  uint32_t sockets;  
  bool ht;
};

void init_cpu_info(struct cpuInfo* cpu) {
  cpu->AVX    = false;
  cpu->AVX2   = false;
  cpu->AVX512 = false;
  cpu->SSE    = false;
  cpu->SSE2   = false;
  cpu->SSE3   = false;
  cpu->SSSE3  = false;
  cpu->SSE4a  = false;
  cpu->SSE4_1 = false;
  cpu->SSE4_2 = false;
  cpu->FMA3   = false;
  cpu->FMA4   = false;
  cpu->AES    = false;
  cpu->SHA    = false;
}

void get_cpu_vendor_internal(char* name, uint32_t ebx,uint32_t ecx,uint32_t edx) {
  name[__COUNTER__] = ebx       & MASK;
  name[__COUNTER__] = (ebx>>8)  & MASK;
  name[__COUNTER__] = (ebx>>16) & MASK;
  name[__COUNTER__] = (ebx>>24) & MASK;

  name[__COUNTER__] = edx       & MASK;
  name[__COUNTER__] = (edx>>8)  & MASK;
  name[__COUNTER__] = (edx>>16) & MASK;
  name[__COUNTER__] = (edx>>24) & MASK;

  name[__COUNTER__] = ecx       & MASK;
  name[__COUNTER__] = (ecx>>8)  & MASK;
  name[__COUNTER__] = (ecx>>16) & MASK;
  name[__COUNTER__] = (ecx>>24) & MASK;
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
  while (*str != '\0') {
    while (*str == ' ' && *(str + 1) == ' ') str++;
    *dest++ = *str++;
  }
  *dest = '\0';
  
  return name;
}

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  init_cpu_info(cpu);
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
  get_cpu_vendor_internal(name, ebx, ecx, edx);
  
  if(strcmp(VENDOR_INTEL_STRING,name) == 0)
    cpu->cpu_vendor = VENDOR_INTEL;
  else if (strcmp(VENDOR_AMD_STRING,name) == 0)
    cpu->cpu_vendor = VENDOR_AMD;  
  else {
    cpu->cpu_vendor = VENDOR_INVALID;
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
    cpu->SSE    = (edx & ((int)1 << 25)) != 0;
    cpu->SSE2   = (edx & ((int)1 << 26)) != 0;
    cpu->SSE3   = (ecx & ((int)1 <<  0)) != 0;

    cpu->SSSE3  = (ecx & ((int)1 <<  9)) != 0;
    cpu->SSE4_1 = (ecx & ((int)1 << 19)) != 0;
    cpu->SSE4_2 = (ecx & ((int)1 << 20)) != 0;

    cpu->AES    = (ecx & ((int)1 << 25)) != 0;

    cpu->AVX    = (ecx & ((int)1 << 28)) != 0;
    cpu->FMA3   = (ecx & ((int)1 << 12)) != 0;
  }
  else {
    printWarn("Can't read features information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x00000001, cpu->maxLevels);
  }
  
  if (cpu->maxLevels >= 0x00000007){
    eax = 0x00000007;
    ecx = 0x00000000;
    cpuid(&eax, &ebx, &ecx, &edx);
    cpu->AVX2         = (ebx & ((int)1 <<  5)) != 0;
    cpu->SHA          = (ebx & ((int)1 << 29)) != 0;
    cpu->AVX512       = (((ebx & ((int)1 << 16)) != 0) ||
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
    cpu->SSE4a = (ecx & ((int)1 <<  6)) != 0;
    cpu->FMA4  = (ecx & ((int)1 << 16)) != 0;
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

  return cpu;
}

// 80/2/20
struct topology* get_topology_info(struct cpuInfo* cpu) {
  struct topology* topo = malloc(sizeof(struct topology));
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  int32_t type;
  
  if (cpu->maxLevels >= 0x00000001) {
    eax = 0x00000001;
    cpuid(&eax, &ebx, &ecx, &edx);
    topo->ht = edx & (1 << 28);
  }
  else {
    printWarn("Can't read HT information from cpuid (needed level is 0x%.8X, max is 0x%.8X). Assuming HT is disabled", 0x00000001, cpu->maxLevels); 
    topo->ht = false;
  }
    
  switch(cpu->cpu_vendor) {
    case VENDOR_INTEL:  
      if (cpu->maxLevels >= 0x0000000B) { 
        eax = 0x0000000B;
        ecx = 0x00000000;
        cpuid(&eax, &ebx, &ecx, &edx);
        type = (ecx >> 8) & 0xFF;
        if (type != 1) {
          printBug("Unexpected type in cpuid 0x0000000B (expected 1, got %d)", type);     
          return NULL;
        }        
        topo->smt = ebx & 0xFFFF;                
  
        eax = 0x0000000B;
        ecx = 0x00000001;
        cpuid(&eax, &ebx, &ecx, &edx); 
        type = (ecx >> 8) & 0xFF;
        if (type < 2) {       
          printBug("Unexpected type in cpuid 0x0000000B (expected < 2, got %d)", type);     
          return NULL;
        }
        topo->logical_cores = ebx & 0xFFFF;
        topo->physical_cores = topo->logical_cores / topo->smt;
      }
      else {
        printWarn("Can't read topology information from cpuid (needed level is 0x%.8X, max is 0x%.8X)", 0x0000000B, cpu->maxLevels); 
        topo->physical_cores = 1;
        topo->logical_cores = 1;
        topo->smt = 1;
      }
      break;
    case VENDOR_AMD:  
      if (cpu->maxExtendedLevels >= 0x80000008) {
        eax = 0x80000008;  
        cpuid(&eax, &ebx, &ecx, &edx);        
        topo->logical_cores = (ecx & 0xFF) + 1;
 
        if (cpu->maxExtendedLevels >= 0x8000001E) {
          eax = 0x8000001E;  
          cpuid(&eax, &ebx, &ecx, &edx);
          topo->smt = ((ebx >> 8) & 0x03) + 1;          
        }
        else {
          printWarn("Can't read topology information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x8000001E, cpu->maxLevels); 
          topo->smt = 1;    
        }
        topo->physical_cores = topo->logical_cores / topo->smt;
      }
      else {
        printWarn("Can't read topology information from cpuid (needed extended level is 0x%.8X, max is 0x%.8X)", 0x80000008, cpu->maxLevels); 
        topo->physical_cores = 1;
        topo->logical_cores = 1;
        topo->smt = 1;    
      }
      break;
    default:
      printBug("Cant get topology because VENDOR is empty");
      return NULL;
  }
  
  // Ask the OS the total number of cores it sees
  // If we have one socket, it will be same as the cpuid,
  // but in dual socket it will not!
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
  topo->sockets = topo->total_cores / topo->smt / topo->physical_cores; // Idea borrowed from lscpu
  
  return topo;
}

struct cache* get_cache_info(struct cpuInfo* cpu) {
  struct cache* cach = malloc(sizeof(struct cache));    
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  uint32_t level;

  // We use standart 0x00000004 for Intel
  // We use extended 0x8000001D for AMD
  if(cpu->cpu_vendor == VENDOR_INTEL) {
    level = 0x00000004;
    if(cpu->maxLevels < level) {
      printErr("Can't read cache information from cpuid (needed level is %d, max is %d)", level, cpu->maxLevels);    
      return NULL;
    }
  }
  else {
    level = 0x8000001D;
    if(cpu->maxExtendedLevels < level) {
      printErr("Can't read cache information from cpuid (needed extended level is %d, max is %d)", level, cpu->maxExtendedLevels);    
      return NULL;
    }
  }
  
  // We suppose there are 4 caches (at most)
  for(int i=0; i < 4; i++) {
    eax = level; // get cache info
    ebx = 0;
    ecx = i; // cache id
    edx = 0;
      
    cpuid(&eax, &ebx, &ecx, &edx); 
      
    int32_t cache_type = eax & 0x1F;
      
    // If its 0, we tried fetching a non existing cache
    if (cache_type > 0) {
      int32_t cache_level = (eax >>= 5) & 0x7;
      int32_t cache_is_self_initializing = (eax >>= 3) & 0x1; // does not need SW initialization
      int32_t cache_is_fully_associative = (eax >>= 1) & 0x1;
      uint32_t cache_sets = ecx + 1;
      uint32_t cache_coherency_line_size = (ebx & 0xFFF) + 1;
      uint32_t cache_physical_line_partitions = ((ebx >>= 12) & 0x3FF) + 1;
      uint32_t cache_ways_of_associativity = ((ebx >>= 10) & 0x3FF) + 1;
        
      int32_t cache_total_size = cache_ways_of_associativity * cache_physical_line_partitions * cache_coherency_line_size * cache_sets;  
        
      switch (cache_type) {
        case 1: // Data Cache (We assume this is L1d)
          if(cache_level != 1) {
            printBug("Found data cache at level %d (expected 1)", cache_level);
            return NULL;
          }
          cach->L1d = cache_total_size; 
          break;
            
        case 2: // Instruction Cache (We assume this is L1i)
          if(cache_level != 1) {
            printBug("Found instruction cache at level %d (expected 1)", cache_level);
            return NULL;
          }
          cach->L1i = cache_total_size;
          break;
          
        case 3: // Unified Cache (This may be L2 or L3)
          if(cache_level == 2) cach->L2 = cache_total_size;
          else if(cache_level == 3) cach->L3 = cache_total_size;
          else {
            printBug("Found unified cache at level %d (expected == 2 or 3)", cache_level);
            return NULL;
          }
          break;
          
        default: // Unknown Type Cache
          printBug("Unknown Type Cache found at ID %d", i);
          return NULL;
      }
    }    
    else if(i == 2) cach->L2 = UNKNOWN;
    else if(i == 3) cach->L3 = UNKNOWN; 
    else {
      printBug("Could not find cache ID %d", i);
      return NULL;    
    }    
  }
  
  // Sanity checks. If we read values greater than this, they can't be valid ones
  // The values were chosen by me
  if(cach->L1i > 64 * 1024) {
    printBug("Invalid L1i size: %dKB", cach->L1i/1024);
    return NULL;
  }
  if(cach->L1d > 64 * 1024) {
    printBug("Invalid L1d size: %dKB", cach->L1d/1024);
    return NULL;
  }
  if(cach->L2 != UNKNOWN && cach->L2 > 2 * 1048576) {
    printBug("Invalid L2 size: %dMB", cach->L2/(1048576));
    return NULL;
  }
  if(cach->L3 != UNKNOWN && cach->L3 > 100 * 1048576) {
    printBug("Invalid L3 size: %dMB", cach->L3/(1048576));
    return NULL;
  }
  
  return cach;
}

struct frequency* get_frequency_info(struct cpuInfo* cpu) {
  struct frequency* freq = malloc(sizeof(struct frequency));
  
  if(cpu->maxLevels < 0x16) {
    #ifdef _WIN32
      printErr("Can't read frequency information from cpuid (needed level is %d, max is %d)", 0x16, cpu->maxLevels);
      freq->base = UNKNOWN;
      freq->max = UNKNOWN;
    #else
      printWarn("Can't read frequency information from cpuid (needed level is %d, max is %d). Using udev", 0x16, cpu->maxLevels);
      freq->base = UNKNOWN;
      freq->max = get_max_freq_from_file();
    #endif
  }
  else {
    uint32_t eax = 0x16;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;
    
    cpuid(&eax, &ebx, &ecx, &edx);
    
    freq->base = eax;
    freq->max = ebx;    
  }
  
  return freq;
}

uint32_t get_nsockets(struct topology* topo) {
  return topo->sockets;    
}

int64_t get_freq(struct frequency* freq) {
  return freq->max;
}

VENDOR get_cpu_vendor(struct cpuInfo* cpu) {
  return cpu->cpu_vendor;
}

void debug_cpu_info(struct cpuInfo* cpu) {
  printf("AVX=%s\n", cpu->AVX ? "true" : "false");
  printf("AVX2=%s\n", cpu->AVX2 ? "true" : "false");
  printf("AVX512=%s\n\n", cpu->AVX512 ? "true" : "false");

  printf("SSE=%s\n", cpu->SSE ? "true" : "false");
  printf("SSE2=%s\n", cpu->SSE2 ? "true" : "false");
  printf("SSE3=%s\n", cpu->SSE3 ? "true" : "false");
  printf("SSSE3=%s\n", cpu->SSSE3 ? "true" : "false");
  printf("SSE4a=%s\n", cpu->SSE4a ? "true" : "false");
  printf("SSE4_1=%s\n", cpu->SSE4_1 ? "true" : "false");
  printf("SSE4_2=%s\n\n", cpu->SSE4_2 ? "true" : "false");

  printf("FMA3=%s\n", cpu->FMA3 ? "true" : "false");
  printf("FMA4=%s\n\n", cpu->FMA4 ? "true" : "false");

  printf("AES=%s\n", cpu->AES ? "true" : "false");
  printf("SHA=%s\n", cpu->SHA ? "true" : "false");
}

void debug_cache(struct cache* cach) {
  printf("L1i=%dB\n",cach->L1i);
  printf("L1d=%dB\n",cach->L1d);
  printf("L2=%dB\n",cach->L2);
  printf("L3=%dB\n",cach->L3);
}

void debug_frequency(struct frequency* freq) {
  #ifdef _WIN32
    printf("maxf=%I64d Mhz\n",freq->max);
    printf("basef=%I64d Mhz\n",freq->base);
  #else
    printf("maxf=%ld Mhz\n",freq->max);
    printf("basef=%ld Mhz\n",freq->base);
  #endif
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
  if(freq == UNKNOWN) {
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
    return string;
  }

  double flops = topo->physical_cores*(freq*1000000);
  
  // Intel USUALLY has two VPUs. I have never seen an AMD 
  // with two VPUs.
  if(cpu->cpu_vendor == VENDOR_INTEL) flops = flops * 2; 

  if(cpu->FMA3 || cpu->FMA4)
    flops = flops*2;

  if(cpu->AVX512)
    flops = flops*16;
  else if(cpu->AVX || cpu->AVX2)
    flops = flops*8;
  else if(cpu->SSE)
    flops = flops*4;

  if(flops >= (double)1000000000000.0)
    snprintf(string,size,"%.2f TFLOP/s",flops/1000000000000);
  else if(flops >= 1000000000.0)
    snprintf(string,size,"%.2f GFLOP/s",flops/1000000000);
  else
    snprintf(string,size,"%.2f MFLOP/s",flops/1000000);
  return string;
}

char* get_str_topology(struct topology* topo, bool dual_socket) {
  char* string;
  if(topo->smt > 1) {
    //3 for digits, 8 for ' cores (', 3 for digits, 9 for ' threads)'
    uint32_t size = 3+8+3+9+1;
    string = malloc(sizeof(char)*size);
    if(dual_socket)
      snprintf(string, size, "%d cores (%d threads)",topo->physical_cores * topo->sockets, topo->logical_cores * topo->sockets);
    else
      snprintf(string, size, "%d cores (%d threads)",topo->physical_cores,topo->logical_cores);
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

char* get_str_sockets(struct topology* topo) {
  char* string = malloc(sizeof(char) * 2);
  int32_t sanity_ret = snprintf(string, 2, "%d", topo->sockets);
  if(sanity_ret < 0) {
    printBug("get_str_sockets: snprintf returned a negative value for input: '%d'", topo->sockets);
    return NULL;    
  }
  return string;
}

char* get_str_cpu_name(struct cpuInfo* cpu) {
  return cpu->cpu_name;    
}

char* get_str_avx(struct cpuInfo* cpu) {
  //If all AVX are available, it will use up to 15
  char* string = malloc(sizeof(char)*15+1);
  if(!cpu->AVX)
    snprintf(string,2+1,"No");
  else if(!cpu->AVX2)
    snprintf(string,3+1,"AVX");
  else if(!cpu->AVX512)
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

  if(cpu->SSE) {
      snprintf(string+last,SSE_sl+1,"SSE,");
      last+=SSE_sl;
  }
  if(cpu->SSE2) {
      snprintf(string+last,SSE2_sl+1,"SSE2,");
      last+=SSE2_sl;
  }
  if(cpu->SSE3) {
      snprintf(string+last,SSE3_sl+1,"SSE3,");
      last+=SSE3_sl;
  }
  if(cpu->SSSE3) {
      snprintf(string+last,SSSE3_sl+1,"SSSE3,");
      last+=SSSE3_sl;
  }
  if(cpu->SSE4a) {
      snprintf(string+last,SSE4a_sl+1,"SSE4a,");
      last+=SSE4a_sl;
  }
  if(cpu->SSE4_1) {
      snprintf(string+last,SSE4_1_sl+1,"SSE4_1,");
      last+=SSE4_1_sl;
  }
  if(cpu->SSE4_2) {
      snprintf(string+last,SSE4_2_sl+1,"SSE4_2,");
      last+=SSE4_2_sl;
  }

  //Purge last comma
  string[last-1] = '\0';
  return string;
}

char* get_str_fma(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*9+1);
  if(!cpu->FMA3)
    snprintf(string,2+1,"No");
  else if(!cpu->FMA4)
    snprintf(string,4+1,"FMA3");
  else
    snprintf(string,9+1,"FMA3,FMA4");

  return string;
}

char* get_str_aes(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*3+1);
  if(cpu->AES)
    snprintf(string,3+1,STRING_YES);
  else
    snprintf(string,2+1,STRING_NO);
  return string;
}

char* get_str_sha(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*3+1);
  if(cpu->SHA)
    snprintf(string,3+1,STRING_YES);
  else
    snprintf(string,2+1,STRING_NO);
  return string;
}

int32_t get_value_as_smallest_unit(char ** str, uint32_t value) {
  int32_t sanity_ret;    
  *str = malloc(sizeof(char)* 11); //8 for digits, 2 for units

  if(value/1024 >= 1024)
    sanity_ret = snprintf(*str, 10,"%.4g"STRING_MEGABYTES, (double)value/(1<<20));    
  else
    sanity_ret = snprintf(*str, 10,"%.4g"STRING_KILOBYTES, (double)value/(1<<10));  
  
  return sanity_ret;
}

// String functions 
char* get_str_cache_two(int32_t cache_size, uint32_t physical_cores) {
  // 4 for digits, 2 for units, 2 for ' (', 3 digits, 2 for units and 7 for ' Total)'
  uint32_t max_size = 4+2 + 2 + 4+2 + 7 + 1;
  int32_t sanity_ret;
  char* string = malloc(sizeof(char) * max_size);  
  char* tmp1;
  char* tmp2;  
  int32_t tmp1_len = get_value_as_smallest_unit(&tmp1, cache_size);
  int32_t tmp2_len = get_value_as_smallest_unit(&tmp2, cache_size * physical_cores);
  
  if(tmp1_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf returned a negative value for input: %d\n", cache_size);
    return NULL;    
  }
  if(tmp2_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf returned a negative value for input: %d\n", cache_size * physical_cores);
    return NULL;    
  }
      
  uint32_t size = tmp1_len + 2 + tmp2_len + 7 + 1;
  sanity_ret = snprintf(string, size, "%s (%s Total)", tmp1, tmp2);  
  
  if(sanity_ret < 0) {
    printBug("get_str_cache_two: snprintf returned a negative value for input: '%s' and '%s'\n", tmp1, tmp2);
    return NULL;    
  }
  
  free(tmp1);
  free(tmp2);
  return string;
}

char* get_str_cache_one(int32_t cache_size) {
  // 4 for digits, 2 for units, 2 for ' (', 3 digits, 2 for units and 7 for ' Total)'
  uint32_t max_size = 4+2 + 1;
  int32_t sanity_ret;
  char* string = malloc(sizeof(char) * max_size);  
  char* tmp;
  int32_t tmp_len = get_value_as_smallest_unit(&tmp, cache_size);
  
  if(tmp_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf returned a negative value for input: %d", cache_size);
    return NULL;    
  }
      
  uint32_t size = tmp_len + 1;
  sanity_ret = snprintf(string, size, "%s", tmp);
  
  if(sanity_ret < 0) {
    printBug("get_str_cache_one: snprintf returned a negative value for input: '%s'", tmp);
    return NULL;    
  }
  free(tmp);
  return string;
}

char* get_str_cache(int32_t cache_size, struct topology* topo, bool llc) {
  if(topo->sockets == 1) {
    if(llc)
      return get_str_cache_one(cache_size);
    else
      return get_str_cache_two(cache_size, topo->physical_cores);
  }
  else {
    if(llc)
      return get_str_cache_two(cache_size, topo->sockets);
    else
      return get_str_cache_two(cache_size, topo->physical_cores * topo->sockets);
  }
}

char* get_str_l1i(struct cache* cach, struct topology* topo) {
  return get_str_cache(cach->L1i, topo, false);
}

char* get_str_l1d(struct cache* cach, struct topology* topo) {
  return get_str_cache(cach->L1d, topo, false);
}

char* get_str_l2(struct cache* cach, struct topology* topo) {
  if(cach->L2 == UNKNOWN) {
    char* string = malloc(sizeof(char) * 5);
    snprintf(string, 5, STRING_NONE);
    return string;
  }
  return get_str_cache(cach->L2, topo, false);
}

char* get_str_l3(struct cache* cach, struct topology* topo) {
  if(cach->L3 == UNKNOWN) {
    char* string = malloc(sizeof(char) * 5);
    snprintf(string, 5, STRING_NONE);
    return string;
  }
  return get_str_cache(cach->L3, topo, true);
}

char* get_str_freq(struct frequency* freq) {
  //Max 3 digits and 3 for '(M/G)Hz' plus 1 for '\0'
  uint32_t size = (4+3+1);
  assert(strlen(STRING_UNKNOWN)+1 <= size);
  char* string = malloc(sizeof(char)*size);
  if(freq->max == UNKNOWN)
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
  else if(freq->max >= 1000)
    snprintf(string,size,"%.2f"STRING_GIGAHERZ,(float)(freq->max)/1000);
  else
    snprintf(string,size,"%.2f"STRING_MEGAHERZ,(float)(freq->max));
  return string;
}

void print_levels(struct cpuInfo* cpu, char* cpu_name) {
  printf("%s\n", cpu_name);
  printf("- Max standart level: 0x%.8X\n", cpu->maxLevels);
  printf("- Max extended level: 0x%.8X\n", cpu->maxExtendedLevels);
}

void free_topo_struct(struct topology* topo) {
  free(topo);
}

void free_cache_struct(struct cache* cach) {
  free(cach);
}

void free_freq_struct(struct frequency* freq) {
  free(freq);
}
