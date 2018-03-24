#include <stdio.h>
#include <stdlib.h>
#include "01h.h"
#include "02h.h"
#include "extended.h"

/***
SAMPLE OUTPUT

Name:        Intel Core i7-4790K
Arch:        64 Bits
Frecuency:   4.0 GHz
NºCores:     4 cores(8 threads)
AXV:         AVX,AVX2
SSE:         SSE,SSE2,SSE4.1,SSE4.2
FMA:         FMA3
AES:         Yes
SHA:         No
L1 Size:     32KB(Data)32KB(Instructions)
L2 Size:     512KB
L3 Size:     8MB
Peak FLOPS:  512 GFLOP/s(in simple precision)

***/

int main() {
  struct level2* level2 = fillLevel2(level2);
  debugLevel2(level2);
  freeLevel2(level2);

  struct cpuInfo* cpu = getCPUInfo(cpu);
  debugCpuInfo(cpu);
  free(cpu);

  printf("%s\n",getCPUName());
}
