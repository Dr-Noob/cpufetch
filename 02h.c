#include <stdlib.h>
#include <stdio.h>
#include "02h.h"
#include "cpuid.h"

#define MASK1 0xFF;
#define MASK2 0xFF00;
#define MASK3 0xFF0000;
#define MASK4 0xFF000000;
#define INVALID -1

struct cache {
  int L1d;
  int L1i;
  int L2;
  int L3;
};

struct TLB {
  int TLBd;
  int TLBi;
};

//http://www.sandpile.org/x86/cpuid.htm
//http://www.hugi.scene.org/online/coding/hugi%2016%20-%20corawhd4.htm

/*** ACCORDING TO CPUID TABLE ***/
//TO BE IMPLEMENTED
void fillWithDescriptor(unsigned int desc, struct level2* data) {
  if(desc == 0x1)
  {
    data->tlb->TLBi = 4096;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x3)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x4)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x5)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x6)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x8)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0xA)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0xC)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0xD)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x21)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x22)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x23)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x25)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x29)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2C)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x30)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x39)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x3A)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x3C)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x3D)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x3E)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x40)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x41)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x42)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x43)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x44)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x45)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x46)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x47)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x48)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x49)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x4A)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x4B)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x4C)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x4D)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x4E)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x4E)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else if (desc == 0x2)
  {
    data->tlb->TLBi = 4194304;
  }
  else
  {
    printf("ERROR: Invalid descriptor(0x%x)\n",desc);
  }
}

void fillData(unsigned int descriptors[16], struct level2* data) {
  //Initialize structures
  data->cache->L1d = INVALID;
  data->cache->L1i = INVALID;
  data->cache->L2 = INVALID;
  data->cache->L3 = INVALID;

  data->tlb->TLBd = INVALID;
  data->tlb->TLBi = INVALID;

  //Fill data for each descriptor
  for(int i=0;i<16;i++) {
    if(descriptors[i] != 0x0)
      fillWithDescriptor(descriptors[i],data);
  }
}

struct level2* fillLevel2(struct level2* data) {
  data = malloc(sizeof(struct level2));
  data->cache = malloc(sizeof(struct cache));
  data->tlb = malloc(sizeof(struct TLB));

  unsigned eax, ebx, ecx, edx;
  eax = 2;
  cpuid(&eax, &ebx, &ecx, &edx);

  unsigned int desc[16];

  /***

  Aplying MASK to register will give us 8 bits which represent descriptors

  Byte1 Byte2 Byte3 Byte4 -> unsigned integer

  desc1 = Byte1
  desc2 = Byte2
  desc3 = Byte3
  desc4 = Byte4

  ***/

  desc[0] = eax & MASK1;
  desc[1] = eax & MASK2;
  desc[2] = eax & MASK3;
  desc[3] = eax & MASK4;

  desc[4] = ebx & MASK1;
  desc[5] = ebx & MASK2;
  desc[6] = ebx & MASK3;
  desc[7] = ebx & MASK4;

  desc[8] = ecx & MASK1;
  desc[9] = ecx & MASK2;
  desc[10] = ecx & MASK3;
  desc[11] = ecx & MASK4;

  desc[12] = edx & MASK1;
  desc[13] = edx & MASK2;
  desc[14] = edx & MASK3;
  desc[15] = edx & MASK4;

  fillData(desc, data);

  return data;
}

void debugCachex(struct cache* cach) {
  printf("L1d=%d\n", cach->L1d);
  printf("L1i=%d\n", cach->L1i);
  printf("L2=%d\n", cach->L2);
  printf("L3=%d\n", cach->L3);
}

void debugTLB(struct TLB* tlb) {
  printf("TLBd=%d\n", tlb->TLBd);
  printf("TLBi=%d\n", tlb->TLBi);
}

void debugLevel2(struct level2* data) {
  debugCachex(data->cache);
  debugTLB(data->tlb);
}

void freeLevel2(struct level2* data) {
  free(data->cache);
  free(data->tlb);
  free(data);
}
