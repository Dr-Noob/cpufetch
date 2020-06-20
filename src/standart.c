#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "standart.h"
#include "cpuid.h"
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

#define UNKNOWN -1

/*
 * cpuid reference: http://www.sandpile.org/x86/cpuid.htm
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
  
  int nThreads;
  // Threads per core
  int HT;
  //  Max cpuids levels
  unsigned int maxLevels;
  // Max cpuids extended levels
  unsigned int maxExtendedLevels;
};

struct cache {
  int L1i;
  int L1d;
  int L2;
  int L3;
};

struct frequency {
  long base;
  long max;
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

#define MASK 0xFF
VENDOR get_cpu_vendor_internal(unsigned eax,unsigned ebx,unsigned ecx,unsigned edx) {
  char name[13];
  memset(name,0,13);
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

  if(strcmp(VENDOR_INTEL_STRING,name) == 0)
    return VENDOR_INTEL;

  else if (strcmp(VENDOR_AMD_STRING,name) == 0)
    return VENDOR_AMD;

  return VENDOR_INVALID;
}

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  init_cpu_info(cpu);

  unsigned int eax = 0;
  unsigned int ebx = 0;
  unsigned int ecx = 0;
  unsigned int edx = 0;

  //Get max cpuid level
  eax = 0x0000000;
  cpuid(&eax, &ebx, &ecx, &edx);
  cpu->maxLevels = eax;

  //Fill vendor
  cpu->cpu_vendor = get_cpu_vendor_internal(eax,ebx,ecx,edx);
  if(cpu->cpu_vendor == VENDOR_INVALID) {
    printf("ERROR: CPU vendor is neither AMD nor INTEL\n");
    return NULL;
  }

  //Get max extended level
  cpuid(&eax, &ebx, &ecx, &edx);
  cpu->maxExtendedLevels = eax;

  //Fill cores and threads
  cpu->nThreads = sysconf(_SC_NPROCESSORS_ONLN);

  //Always check we can fetch data
  if (cpu->maxLevels >= 0x0000000B) {
    eax = 0x0000000B;
    ecx = 0x00000000;
    cpuid(&eax, &ebx, &ecx, &edx);
    cpu->HT = ebx & 0xF;
    if(cpu->HT == 0) {
      //AMD should not work with this, returning 0
      //Suppose we have 1
      cpu->HT = 1;
    }
  }
  else {
    //We cant afford this check, assume 1
    cpu->HT = 1;
  }

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
  if (cpu->maxExtendedLevels >= 0x80000001){
      eax = 0x80000001;
      cpuid(&eax, &ebx, &ecx, &edx);
      cpu->SSE4a = (ecx & ((int)1 <<  6)) != 0;
      cpu->FMA4  = (ecx & ((int)1 << 16)) != 0;
  }

  return cpu;
}

// see https://stackoverflow.com/questions/12594208/c-program-to-determine-levels-size-of-cache
struct cache* get_cache_info(struct cpuInfo* cpu) {
  struct cache* cach = malloc(sizeof(struct cache));  
  unsigned int eax, ebx, ecx, edx;
  
  // We suppose there are 4 caches (at most)
  for(int i=0; i < 4; i++) {
    eax = 4; // get cache info
    ebx = 0;
    ecx = i; // cache id
    edx = 0;
      
    cpuid(&eax, &ebx, &ecx, &edx); 
      
    int cache_type = eax & 0x1F;
      
    // If its 0, we tried fetching a non existing cache
    if (cache_type > 0) {
      int cache_level = (eax >>= 5) & 0x7;
      int cache_is_self_initializing = (eax >>= 3) & 0x1; // does not need SW initialization
      int cache_is_fully_associative = (eax >>= 1) & 0x1;
      unsigned int cache_sets = ecx + 1;
      unsigned int cache_coherency_line_size = (ebx & 0xFFF) + 1;
      unsigned int cache_physical_line_partitions = ((ebx >>= 12) & 0x3FF) + 1;
      unsigned int cache_ways_of_associativity = ((ebx >>= 10) & 0x3FF) + 1;
        
      int cache_total_size = cache_ways_of_associativity * cache_physical_line_partitions * cache_coherency_line_size * cache_sets;  
        
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
  }
  
  return cach;
}

struct frequency* get_frequency_info(struct cpuInfo* cpu) {
  struct frequency* freq = malloc(sizeof(struct frequency));
  
  if(cpu->maxLevels < 0x16) {
    printErr("Can't read frequency information from cpuid (needed level is %d, max is %d)", 0x16, cpu->maxLevels);
    freq->base = UNKNOWN;
    freq->max = UNKNOWN;
  }
  else {
    unsigned int eax = 0x16;
    unsigned int ebx = 0;
    unsigned int ecx = 0;
    unsigned int edx = 0;
    
    cpuid(&eax, &ebx, &ecx, &edx);
    
    freq->base = eax;
    freq->max = ebx;
  }
  
  return freq;
}

long get_freq(struct frequency* freq) {
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
  printf("max f=%ldMhz\n",freq->max);
  printf("base f=%ldMhz\n",freq->base);
}

/*** STRING FUNCTIONS ***/

char* get_str_peak_performance(struct cpuInfo* cpu, long freq) {
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
  unsigned int size = 7+6+1+1;
  assert(strlen(STRING_UNKNOWN)+1 <= size);
  char* string = malloc(sizeof(char)*size);

  //First check we have consistent data
  if(freq == UNKNOWN) {
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
    return string;
  }

  float flops = (cpu->nThreads/cpu->HT)*(freq*1000000)*2;

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

char* get_str_ncores(struct cpuInfo* cpu) {
  if(cpu->HT > 1) {
    //2(N.Cores)7(' cores(')3(N.Threads)9(' threads)')
    int size = 2+7+3+9+1;
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%d cores(%d threads)",cpu->nThreads/cpu->HT,cpu->nThreads);
    return string;
  }
  else {
    char* string = malloc(sizeof(char)*2+7+1);
    snprintf(string,2+7+1,"%d cores",cpu->nThreads);
    return string;
  }

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
  int last = 0;
  int SSE_sl = 4;
  int SSE2_sl = 5;
  int SSE3_sl = 5;
  int SSSE3_sl = 6;
  int SSE4a_sl = 6;
  int SSE4_1_sl = 7;
  int SSE4_2_sl = 7;
  char* string = malloc(sizeof(char)*SSE_sl+SSE2_sl+SSE3_sl+SSSE3_sl+SSE4a_sl+SSE4_1_sl+SSE4_2_sl+1);

  if(!cpu->SSE) {
      snprintf(string+last,SSE_sl+1,"SSE,");
      last+=SSE_sl;
  }
  if(!cpu->SSE2) {
      snprintf(string+last,SSE2_sl+1,"SSE2,");
      last+=SSE2_sl;
  }
  if(!cpu->SSE3) {
      snprintf(string+last,SSE3_sl+1,"SSE3,");
      last+=SSE3_sl;
  }
  if(!cpu->SSSE3) {
      snprintf(string+last,SSSE3_sl+1,"SSSE3,");
      last+=SSSE3_sl;
  }
  if(!cpu->SSE4a) {
      snprintf(string+last,SSE4a_sl+1,"SSE4a,");
      last+=SSE4a_sl;
  }
  if(!cpu->SSE4_1) {
      snprintf(string+last,SSE4_1_sl+1,"SSE4_1,");
      last+=SSE4_1_sl;
  }
  if(!cpu->SSE4_2) {
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

// String functions
char* get_str_l1(struct cache* cach) {
  //Max 2 digits,2 for 'KB',3 for '(D)' and '(I)'
  int size = (2*(2+2)+6+1);
  char* string = malloc(sizeof(char)*size);
  snprintf(string,size,"%d"STRING_KILOBYTES"(D)%d"STRING_KILOBYTES"(I)",cach->L1d/1024,cach->L1i/1024);
  return string;
}

char* get_str_l2(struct cache* cach) {
  if(cach->L2 == UNKNOWN) {
    char* string = malloc(sizeof(char)*5);
    snprintf(string,5,STRING_NONE);
    return string;
  }
  else {
    //Max 4 digits and 2 for 'KB'
    int size = (4+2+1);
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%d"STRING_KILOBYTES,cach->L2/1024);
    return string;
  }
}

char* get_str_l3(struct cache* cach) {
  if(cach->L3 == UNKNOWN) {
    char* string = malloc(sizeof(char)*5);
    snprintf(string,5,STRING_NONE);
    return string;
  }
  else {
    //Max 4 digits and 2 for 'KB'
    int size = (4+2+1);
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%d"STRING_KILOBYTES,cach->L3/1024);
    return string;
  }
}

char* get_str_freq(struct frequency* freq) {
  //Max 3 digits and 3 for '(M/G)Hz' plus 1 for '\0'
  unsigned int size = (4+3+1);
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

void free_cache_struct(struct cache* cach) {
  free(cach);
}

void free_freq_struct(struct frequency* freq) {
  free(freq);
}
