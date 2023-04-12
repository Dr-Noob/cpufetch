#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../common/global.h"
#include "../common/udev.h"
#include "udev.h"
#include "uarch.h"
#include "soc.h"

struct frequency* get_frequency_info(uint32_t core) {
  struct frequency* freq = emalloc(sizeof(struct frequency));

  freq->base = UNKNOWN_DATA;
  freq->max = get_max_freq_from_file(core);

  return freq;
}

int64_t get_peak_performance(struct cpuInfo* cpu) {
  //First check we have consistent data
  if(get_freq(cpu->freq) == UNKNOWN_DATA) {
    return -1;
  }

  int64_t flops = cpu->topo->total_cores * (get_freq(cpu->freq) * 1000000);
  return flops;
}

struct cpuInfo* get_cpu_info(void) {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  //init_cpu_info(cpu);
  struct topology* topo = emalloc(sizeof(struct topology));
  topo->total_cores = get_ncores_from_cpuinfo();
  topo->cach = NULL;
  cpu->topo = topo;

  char* cpuinfo_str = get_uarch_from_cpuinfo();
  cpu->hv = emalloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  cpu->arch = get_uarch_from_cpuinfo_str(cpuinfo_str, cpu);
  cpu->soc = get_soc();
  cpu->freq = get_frequency_info(0);
  cpu->peak_performance = get_peak_performance(cpu);

  return cpu;
}

//TODO: Might be worth refactoring with other archs
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo) {
  uint32_t size = 3+7+1;
  char* string = emalloc(sizeof(char)*size);
  snprintf(string, size, "%d cores", topo->total_cores);

  return string;
}

char* get_str_extensions(struct cpuInfo* cpu) {
  return NULL;
}

void print_debug(struct cpuInfo* cpu) {
  printf("Unimplemented!\n");
}
