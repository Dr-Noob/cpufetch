#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "ppc.h"

#define STRING_UNKNOWN    "Unknown"

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  struct features* feat = malloc(sizeof(struct features));
  cpu->feat = feat;

  bool *ptr = &(feat->AES);
  for(uint32_t i = 0; i < sizeof(struct features)/sizeof(bool); i++, ptr++) {
    *ptr = false;
  }

  cpu->cpu_name = malloc(sizeof(char) * strlen(STRING_UNKNOWN) + 1);
  snprintf(cpu->cpu_name, strlen(STRING_UNKNOWN) + 1, STRING_UNKNOWN);

  return cpu;
}

void print_debug(struct cpuInfo* cpu) {
  printf("TODO\n");
}
