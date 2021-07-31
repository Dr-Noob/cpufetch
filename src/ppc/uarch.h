#ifndef __UARCH__
#define __UARCH__

#include "ppc.h"

struct uarch;

struct uarch* get_uarch_from_auxval();
char* get_str_uarch(struct cpuInfo* cpu);
char* get_str_process(struct cpuInfo* cpu);
void free_uarch_struct(struct uarch* arch);

#endif
