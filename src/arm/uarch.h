#ifndef __UARCH__
#define __UARCH__

#include <stdint.h>

#include "midr.h"

struct uarch* get_uarch_from_midr(uint32_t midr, struct cpuInfo* cpu);
int get_number_of_vpus(struct cpuInfo* cpu);
int get_vpus_width(struct cpuInfo* cpu);
bool has_fma_support(struct cpuInfo* cpu);
char* get_str_uarch(struct cpuInfo* cpu);
void free_uarch_struct(struct uarch* arch);

#endif
