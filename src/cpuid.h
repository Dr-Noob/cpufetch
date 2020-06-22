#ifndef __CPUID__
#define __CPUID__

#include <stdint.h>

void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

#endif
