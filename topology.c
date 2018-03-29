void cpuid(unsigned int *eax, unsigned int *ebx,
                         unsigned int *ecx, unsigned int *edx)
{
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
  unsigned eax, ebx, ecx, edx;

  eax = 0x0000000B;
  ecx = 0x00000000;
  cpuid(&eax, &ebx, &ecx, &edx);

  printf("Threads per core=%d\n", ebx & 0xF);

}
