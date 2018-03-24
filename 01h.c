#include <stdio.h>
#include <stdlib.h>
#include "01h.h"
#include "cpuid.h"

#define BOOLEAN_TRUE 1
#define BOOLEAN_FALSE 0

/***

  MASTER PAGE
  http://www.sandpile.org/x86/cpuid.htm

***/

struct cpuInfo {
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

  int maxLevels;
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

  //Always check we can fetch data
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
