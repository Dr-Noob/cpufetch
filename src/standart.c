#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "standart.h"
#include "cpuid.h"
#include "udev.h"

#define BOOLEAN_TRUE 1
#define BOOLEAN_FALSE 0

/***

  MASTER PAGE
  http://www.sandpile.org/x86/cpuid.htm

***/

struct cpuInfo {
  /*** BOOLEAN VALUES ***/

  /* (256 bits) */
  int AVX;
  int AVX2;

  /* (512 bits) */
  int AVX512F;
  int AVX512CD;
  int AVX512ER;
  int AVX512PF;

  int AVX512VL;
  int AVX512DQ;
  int AVX512BW;

  /* (128 bits) */
  int SSE;
  int SSE2;
  int SSE3;
  int SSSE3;
  int SSE4a;
  int SSE4_1;
  int SSE4_2;

  int FMA3;
  int FMA4;

  int AES;
  int SHA;

  VENDOR cpu_vendor;
  /*** Number of threads ***/
  int nThreads;
  /*** Threads per core(Intel HyperThreading) ***/
  int HT;
  /*** Max CPUIDs levels ***/
  unsigned maxLevels;
  /*** Max CPUIDs extended levels ***/
  unsigned maxExtendedLevels;
};

void initializeCpuInfo(struct cpuInfo* cpu) {
  cpu->AVX         = BOOLEAN_FALSE;
  cpu->AVX2        = BOOLEAN_FALSE;
  cpu->AVX512F     = BOOLEAN_FALSE;
  cpu->AVX512PF    = BOOLEAN_FALSE;
  cpu->AVX512ER    = BOOLEAN_FALSE;
  cpu->AVX512CD    = BOOLEAN_FALSE;
  cpu->AVX512VL    = BOOLEAN_FALSE;
  cpu->AVX512DQ    = BOOLEAN_FALSE;
  cpu->AVX512BW    = BOOLEAN_FALSE;
  cpu->SSE         = BOOLEAN_FALSE;
  cpu->SSE2        = BOOLEAN_FALSE;
  cpu->SSE3        = BOOLEAN_FALSE;
  cpu->SSSE3       = BOOLEAN_FALSE;
  cpu->SSE4a       = BOOLEAN_FALSE;
  cpu->SSE4_1      = BOOLEAN_FALSE;
  cpu->SSE4_2      = BOOLEAN_FALSE;
  cpu->FMA3        = BOOLEAN_FALSE;
  cpu->FMA4        = BOOLEAN_FALSE;
  cpu->AES         = BOOLEAN_FALSE;
  cpu->SHA         = BOOLEAN_FALSE;
}

#define MASK 0xFF
VENDOR getCPUVendor(unsigned eax,unsigned ebx,unsigned ecx,unsigned edx) {
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

struct cpuInfo* getCPUInfo() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  memset(cpu,0,sizeof(struct cpuInfo));
  initializeCpuInfo(cpu);

  unsigned eax = 0;
  unsigned ebx = 0;
  unsigned ecx = 0;
  unsigned edx = 0;

  //Get max cpuid level
  eax = 0x0000000;
  cpuid(&eax, &ebx, &ecx, &edx);
  cpu->maxLevels = eax;

  //Fill vendor
  cpu->cpu_vendor = getCPUVendor(eax,ebx,ecx,edx);
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
    //We can afford this check, assume 1
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
    cpu->SSE4_1  = (ecx & ((int)1 << 19)) != 0;
    cpu->SSE4_2  = (ecx & ((int)1 << 20)) != 0;

    cpu->AES    = (ecx & ((int)1 << 25)) != 0;

    cpu->AVX    = (ecx & ((int)1 << 28)) != 0;
    cpu->FMA3   = (ecx & ((int)1 << 12)) != 0;
  }
  if (cpu->maxLevels >= 0x00000007){
    eax = 0x00000007;
    ecx = 0x00000000;
    cpuid(&eax, &ebx, &ecx, &edx);
    cpu->SHA          = (ebx & ((int)1 << 29)) != 0;

    cpu->AVX2         = (ebx & ((int)1 <<  5)) != 0;

    cpu->AVX512F      = ((ebx & ((int)1 << 16)) != 0);
    cpu->AVX512PF     = ((ebx & ((int)1 << 26)) != 0);
    cpu->AVX512ER     = ((ebx & ((int)1 << 27)) != 0);
    cpu->AVX512CD     = ((ebx & ((int)1 << 28)) != 0);

    cpu->AVX512VL     = ((ebx & ((int)1 << 31)) != 0);
    cpu->AVX512DQ     = ((ebx & ((int)1 << 17)) != 0);
    cpu->AVX512BW     = ((ebx & ((int)1 << 30)) != 0);

    // ((ebx & ((int)1 << 21)) != 0); //AVX512IMFA
    // ((ecx & ((int)1 <<  1)) != 0); //AVX512VBMI
    // ((ecx & ((int)1 <<  6)) != 0); //AVX512VBMI2
    // ((ecx & ((int)1 <<  8)) != 0); //GFNI
    // ((ecx & ((int)1 <<  9)) != 0); //VAES
    // ((ecx & ((int)1 << 10)) != 0); //VPCLMULQDQ
    // ((ecx & ((int)1 << 11)) != 0); //AVX512VNNI
    // ((ecx & ((int)1 << 12)) != 0); //AVX512BITALG
    // ((ecx & ((int)1 << 14)) != 0); //AVX512VPOPCNTDQ
    // ((edx & ((int)1 <<  8)) != 0); //AVX512VP2INTERSECT
    // ecx = 0x00000001;
    // cpuid(&eax, &ebx, &ecx, &edx);
    // ((eax & ((int)1 <<  5)) != 0); //AVX512BF16
  }
  if (cpu->maxExtendedLevels >= 0x80000001){
      eax = 0x80000001;
      cpuid(&eax, &ebx, &ecx, &edx);
      cpu->SSE4a = (ecx & ((int)1 <<  6)) != 0;
      cpu->FMA4  = (ecx & ((int)1 << 16)) != 0;
  }

  return cpu;
}

void debugCpuInfo(struct cpuInfo* cpu) {
  printf("AVX=%s\n", cpu->AVX ? "true" : "false");
  printf("AVX2=%s\n", cpu->AVX2 ? "true" : "false");

  printf("AVX512F =%s\n\n", cpu->AVX512F  ? "true" : "false");
  printf("AVX512PF=%s\n\n", cpu->AVX512PF ? "true" : "false");
  printf("AVX512ER=%s\n\n", cpu->AVX512ER ? "true" : "false");
  printf("AVX512CD=%s\n\n", cpu->AVX512CD ? "true" : "false");

  printf("AVX512VL=%s\n\n", cpu->AVX512VL ? "true" : "false");
  printf("AVX512DQ=%s\n\n", cpu->AVX512DQ ? "true" : "false");
  printf("AVX512BW=%s\n\n", cpu->AVX512BW ? "true" : "false");

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

/*** STRING FUNCTIONS ***/

char* getPeakPerformance(struct cpuInfo* cpu, long freq) {
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

  float flops = (cpu->nThreads/cpu->HT)*freq*2;

  if(cpu->FMA3 || cpu->FMA4)
    flops = flops*2;

  if(cpu->AVX512F)
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

VENDOR getCPUVendorInternal(struct cpuInfo* cpu) {
  return cpu->cpu_vendor;
}

char* getString_NumberCores(struct cpuInfo* cpu) {
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

char* getString_AVX(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*128);
  memset(string, 0, 128);
  char* last = string;
  if(cpu->AVX == BOOLEAN_TRUE)
    last += sprintf(last,"AVX,");
  if(cpu->AVX2 == BOOLEAN_TRUE)
    last += sprintf(last,"AVX2,");
  if(cpu->AVX512F == BOOLEAN_TRUE)
    last += sprintf(last,"AVX512F,");
  if(cpu->AVX512PF == BOOLEAN_TRUE)
    last += sprintf(last,"AVX512PF,");
  if(cpu->AVX512ER == BOOLEAN_TRUE)
    last += sprintf(last,"AVX512ER,");
  if(cpu->AVX512CD == BOOLEAN_TRUE)
    last += sprintf(last,"AVX512CD,");

  if(cpu->AVX512VL == BOOLEAN_TRUE)
    last += sprintf(last,"AVX512VL,");
  if(cpu->AVX512DQ == BOOLEAN_TRUE)
    last += sprintf(last,"AVX512DQ,");
  if(cpu->AVX512BW == BOOLEAN_TRUE)
    last += sprintf(last,"AVX512BW,");

  // Remove trailing comma
  if(last > string) *(last - 1) = 0;
  return string;
}

char* getString_SSE(struct cpuInfo* cpu) {
  int last = 0;
  int SSE_sl = 4;
  int SSE2_sl = 5;
  int SSE3_sl = 5;
  int SSSE3_sl = 6;
  int SSE4a_sl = 6;
  int SSE4_1_sl = 7;
  int SSE4_2_sl = 7;
  char* string = malloc(sizeof(char)*SSE_sl+SSE2_sl+SSE3_sl+SSSE3_sl+SSE4a_sl+SSE4_1_sl+SSE4_2_sl+1);

  if(cpu->SSE == BOOLEAN_TRUE) {
      snprintf(string+last,SSE_sl+1,"SSE,");
      last+=SSE_sl;
  }
  if(cpu->SSE2 == BOOLEAN_TRUE) {
      snprintf(string+last,SSE2_sl+1,"SSE2,");
      last+=SSE2_sl;
  }
  if(cpu->SSE3 == BOOLEAN_TRUE) {
      snprintf(string+last,SSE3_sl+1,"SSE3,");
      last+=SSE3_sl;
  }
  if(cpu->SSSE3 == BOOLEAN_TRUE) {
      snprintf(string+last,SSSE3_sl+1,"SSSE3,");
      last+=SSSE3_sl;
  }
  if(cpu->SSE4a == BOOLEAN_TRUE) {
      snprintf(string+last,SSE4a_sl+1,"SSE4a,");
      last+=SSE4a_sl;
  }
  if(cpu->SSE4_1 == BOOLEAN_TRUE) {
      snprintf(string+last,SSE4_1_sl+1,"SSE4_1,");
      last+=SSE4_1_sl;
  }
  if(cpu->SSE4_2 == BOOLEAN_TRUE) {
      snprintf(string+last,SSE4_2_sl+1,"SSE4_2,");
      last+=SSE4_2_sl;
  }

  //Purge last comma
  string[last-1] = '\0';
  return string;
}

char* getString_FMA(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*9+1);
  if(cpu->FMA3 == BOOLEAN_FALSE)
    snprintf(string,2+1,"No");
  else if(cpu->FMA4 == BOOLEAN_FALSE)
    snprintf(string,4+1,"FMA3");
  else
    snprintf(string,9+1,"FMA3,FMA4");

  return string;
}

char* getString_AES(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*3+1);
  if(cpu->AES == BOOLEAN_TRUE)
    snprintf(string,3+1,STRING_YES);
  else
    snprintf(string,2+1,STRING_NO);
  return string;
}

char* getString_SHA(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*3+1);
  if(cpu->SHA == BOOLEAN_TRUE)
    snprintf(string,3+1,STRING_YES);
  else
    snprintf(string,2+1,STRING_NO);
  return string;
}
