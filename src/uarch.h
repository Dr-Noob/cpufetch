#ifndef __UARCH__
#define __UARCH__

#include <stdint.h>

#include "cpuid.h"

struct uarch;

struct uarch* get_uarch_from_cpuid(struct cpuInfo* cpu, uint32_t ef, uint32_t f, uint32_t em, uint32_t m, int s);
bool vpus_are_AVX512(struct cpuInfo* cpu);
int get_number_of_vpus(struct cpuInfo* cpu);
char* get_str_uarch(struct cpuInfo* cpu);
char* get_str_process(struct cpuInfo* cpu);

#endif
