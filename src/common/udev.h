#ifndef __UDEV__
#define __UDEV__

#include <stdint.h>
#include <stdbool.h>

long get_max_freq_from_file(uint32_t core);
long get_min_freq_from_file(uint32_t core);

#ifdef ARCH_ARM
#define UNKNOWN -1
int get_ncores_from_cpuinfo();
uint32_t get_midr_from_cpuinfo(uint32_t core, bool* success);
#endif

#endif
