#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../common/global.h"
#include "../common/udev.h"

struct frequency* get_frequency_info(uint32_t core) {
  struct frequency* freq = emalloc(sizeof(struct frequency));

  freq->base = UNKNOWN_DATA;
  freq->max = get_max_freq_from_file(core);

  return freq;
}

struct cpuInfo* get_cpu_info(void) {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  //init_cpu_info(cpu);
  int ncores = get_ncores_from_cpuinfo();
  cpu->hv = emalloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  //cpu->soc = get_soc();
  //cpu->peak_performance = get_peak_performance(cpu);
  cpu->peak_performance = 0;
  cpu->freq = get_frequency_info(0);
  cpu->cpu_vendor = CPU_VENDOR_RISCV;

  return cpu;
}

char* get_str_topology(struct cpuInfo* cpu, struct topology* topo) {
  return "Many cores";
}

void print_debug(struct cpuInfo* cpu) {
  printf("Unimplemented!\n");
}
