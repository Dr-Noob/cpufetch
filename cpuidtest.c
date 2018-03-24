static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
                                unsigned int *ecx, unsigned int *edx)
{
        /* ecx is often an input as well as an output. */
        asm volatile("cpuid"
            : "=a" (*eax),
              "=b" (*ebx),
              "=c" (*ecx),
              "=d" (*edx)
            : "0" (*eax), "2" (*ecx));
}

#include <stdio.h>

int main(int argc, char **argv)
{
  unsigned mask1 = 0xFF;
  unsigned mask2 = 0xFF00;
  unsigned mask3 = 0xFF0000;
  unsigned mask4 = 0xFF000000;

  unsigned eax, ebx, ecx, edx;

  eax = 2; /* processor info and feature bits */
  native_cpuid(&eax, &ebx, &ecx, &edx);

  /*
  printf("AL = 0x%x\n",eax & 0xFF);
  printf("0x%x(0x%x)\n",eax, (1 << 31) & eax);
  printf("0x%x(0x%x)\n",ebx, (1 << 31) & ebx);
  printf("0x%x(0x%x)\n",ecx, (1 << 31) & ecx);
  printf("0x%x(0x%x)\n",edx, (1 << 31) & edx);
  */

  printf("eax[0]=0x%x\neax[1]=0x%x\neax[2]=0x%x\neax[3]=0x%x\n",eax & mask1, eax & mask2, eax & mask3, eax & mask4);
  printf("ebx[0]=0x%x\nebx[1]=0x%x\nebx[2]=0x%x\nebx[3]=0x%x\n",ebx & mask1, ebx & mask2, ebx & mask3, ebx & mask4);
  printf("ecx[0]=0x%x\necx[1]=0x%x\necx[2]=0x%x\necx[3]=0x%x\n",ecx & mask1, ecx & mask2, ecx & mask3, ecx & mask4);
  printf("edx[0]=0x%x\nedx[1]=0x%x\nedx[2]=0x%x\nedx[3]=0x%x\n",edx & mask1, edx & mask2, edx & mask3, edx & mask4);


  /*
  if(((eax >> 31) & 1U) == 0)
    printf("eax valid\n");

  if(((ebx >> 31) & 1U) == 0)
    printf("ebx valid\n");

  if(((ecx >> 31) & 1U) == 0)
    printf("ecx valid\n");

  if(((edx >> 31) & 1U) == 0)
    printf("edx valid\n");
  */
}
