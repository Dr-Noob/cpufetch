#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uarch.h"
#include "../common/global.h"

struct uarch* get_uarch_from_cpuid(struct cpuInfo* cpu, uint32_t ef, uint32_t f, uint32_t em, uint32_t m, int s) { return NULL; }
char* get_str_uarch(struct cpuInfo* cpu) { char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "Unknown"); return tmp; }
char* get_str_process(struct cpuInfo* cpu) { char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "Unknown"); return tmp; }
void free_uarch_struct(struct uarch* arch) { }
