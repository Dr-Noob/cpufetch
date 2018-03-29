#include <stdio.h>
#include <stdlib.h>
#include "printer.h"
#include "01h.h"
#include "udev.h"
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
  struct cpuInfo* cpu = getCPUInfo(cpu);
  struct cache* cach = new_cache(cach);
  struct frequency* freq = new_frequency(freq);

  char* cpuName = getString_CPUName();
  char* maxFrequency = getString_MaxFrequency(freq);
  char* nCores = getString_NumberCores(cpu);
  char* avx = getString_AVX(cpu);
  char* sse = getString_SSE(cpu);
  char* fma = getString_FMA(cpu);
  char* aes = getString_AES(cpu);
  char* sha = getString_SHA(cpu);
  char* l1 = getString_L1(cach);
  char* l2 = getString_L2(cach);
  char* l3 = getString_L3(cach);

  printf(TITLE_NAME"%s\n",cpuName);
  printf(TITLE_ARCH"%s\n","x86_64");
  printf(TITLE_FREQUENCY"%s\n",maxFrequency);
  printf(TITLE_NCORES"%s\n",nCores);
  printf(TITLE_AVX"%s\n",avx);
  printf(TITLE_SSE"%s\n",sse);
  printf(TITLE_FMA"%s\n",fma);
  printf(TITLE_AES"%s\n",aes);
  printf(TITLE_SHA"%s\n",sha);
  printf(TITLE_L1"%s\n",l1);
  printf(TITLE_L2"%s\n",l2);
  printf(TITLE_L3"%s\n",l3);
  printf(TITLE_PEAK"%s\n","??? GFLOP/s");

  free(cpuName);
  free(maxFrequency);
  free(nCores);
  free(avx);
  free(sse);
  free(fma);
  free(aes);
  free(sha);
  free(l1);
  free(l2);
  free(l3);

  free(cpu);
  freeCache(cach);
  freeFrequency(freq);

}
