#ifndef __UARCH__
#define __UARCH__

#include <stdint.h>

#include "cpuid.h"

struct uarch;

struct uarch* get_uarch_from_cpuid(struct cpuInfo* cpu, uint32_t dump, uint32_t ef, uint32_t f, uint32_t em, uint32_t m, int s);
char* infer_cpu_name_from_uarch(struct uarch* arch);
bool vpus_are_AVX512(struct cpuInfo* cpu);
bool is_knights_landing(struct cpuInfo* cpu);
int get_number_of_vpus(struct cpuInfo* cpu);
uint32_t get_hybrid_num_cpus(struct uarch* arch);
bool choose_new_intel_logo_uarch(struct cpuInfo* cpu);
char* get_str_uarch(struct cpuInfo* cpu);
char* get_str_process(struct cpuInfo* cpu);
void free_uarch_struct(struct uarch* arch);

#endif
