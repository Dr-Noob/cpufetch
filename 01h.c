#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "01h.h"
#include "cpuid.h"

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
  int AVX512;

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

  /*** Number of threads ***/
  int nThreads;
  /*** Threads per core(Intel HyperThreading) ***/
  int HT;
  /*** Max CPUIDs levels ***/
  int maxLevels;
  /*** Max CPUIDs extended levels ***/
  int maxExtendedLevels;
};

void initializeCpuInfo(struct cpuInfo* cpu) {
  cpu->AVX    = BOOLEAN_FALSE;
  cpu->AVX2   = BOOLEAN_FALSE;
  cpu->AVX512 = BOOLEAN_FALSE;
  cpu->SSE    = BOOLEAN_FALSE;
  cpu->SSE2   = BOOLEAN_FALSE;
  cpu->SSE3   = BOOLEAN_FALSE;
  cpu->SSSE3  = BOOLEAN_FALSE;
  cpu->SSE4a  = BOOLEAN_FALSE;
  cpu->SSE4_1 = BOOLEAN_FALSE;
  cpu->SSE4_2 = BOOLEAN_FALSE;
  cpu->FMA3   = BOOLEAN_FALSE;
  cpu->FMA4   = BOOLEAN_FALSE;
  cpu->AES    = BOOLEAN_FALSE;
  cpu->SHA    = BOOLEAN_FALSE;
}

struct cpuInfo* getCPUInfo() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  initializeCpuInfo(cpu);

  unsigned eax, ebx, ecx, edx;

  eax = 0x0000000;
  cpuid(&eax, &ebx, &ecx, &edx);
  cpu->maxLevels = eax;

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
    cpu->AVX2         = (ebx & ((int)1 <<  5)) != 0;
    cpu->SHA          = (ebx & ((int)1 << 29)) != 0;
    cpu->AVX512       = (((ebx & ((int)1 << 16)) != 0) ||
                        ((ebx & ((int)1 << 28)) != 0) ||
                        ((ebx & ((int)1 << 26)) != 0) ||
                        ((ebx & ((int)1 << 27)) != 0) ||
                        ((ebx & ((int)1 << 31)) != 0) ||
                        ((ebx & ((int)1 << 30)) != 0) ||
                        ((ebx & ((int)1 << 17)) != 0) ||
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

void debugCpuInfo(struct cpuInfo* cpu) {
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
  int size = 7+6+1+1;
  char* string = malloc(sizeof(char)*size);
  float flops = (cpu->nThreads/cpu->HT)*freq*2;

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

//4 cores(8 threads)
char* getString_NumberCores(struct cpuInfo* cpu) {
  if(cpu->HT > 1) {
    //2(N.Cores)7(' cores(')2(N.Threads)9(' threads)')
    int size = 2+7+2+9+1;
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%d cores(%d threads)",cpu->nThreads,cpu->nThreads/cpu->HT);
    return string;
  }
  else {
    char* string = malloc(sizeof(char)*2+1);
    snprintf(string,2+1,"%d cores",cpu->nThreads);
    return string;
  }

}

char* getString_AVX(struct cpuInfo* cpu) {
  //If all AVX are available, it will use up to 15
  char* string = malloc(sizeof(char)*15+1);
  if(cpu->AVX == BOOLEAN_FALSE)
    snprintf(string,2+1,"No");
  else if(cpu->AVX2 == BOOLEAN_FALSE)
    snprintf(string,3+1,"AVX");
  else if(cpu->AVX512 == BOOLEAN_FALSE)
    snprintf(string,8+1,"AVX,AVX2");
  else
    snprintf(string,15+1,"AVX,AVX2,AVX512");

  return string;
}

char* getString_SSE(struct cpuInfo* cpu) {
  char* string = malloc(sizeof(char)*33+1);
  int last = 0;
  int SSE_sl = 4;
  int SSE2_sl = 5;
  int SSE3_sl = 5;
  int SSSE3_sl = 6;
  int SSE4a_sl = 6;
  int SSE4_1_sl = 7;
  int SSE4_2_sl = 7;

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
