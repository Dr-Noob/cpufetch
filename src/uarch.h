#ifndef __UARCH__
#define __UARCH__

#include <stdint.h>

#include "cpuid.h"

struct uarch;

struct uarch* get_uarch_from_cpuid(uint32_t ef, uint32_t f, uint32_t em, uint32_t m, int s);
int get_number_of_vpus(struct cpuInfo* cpu);

#endif
