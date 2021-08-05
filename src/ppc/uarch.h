#ifndef __UARCH__
#define __UARCH__

#include <stdint.h>
#include "ppc.h"

struct uarch;

struct uarch* get_uarch_from_pvr(uint32_t pvr);
bool has_altivec(struct uarch* arch);
bool is_power9(struct uarch* arch);
char* get_str_uarch(struct cpuInfo* cpu);
char* get_str_process(struct cpuInfo* cpu);
void free_uarch_struct(struct uarch* arch);

#endif
