#ifndef __UDEV__
#define __UDEV__

#include <stdint.h>

long get_max_freq_from_file();
long get_min_freq_from_file();

#ifdef ARCH_ARM
int get_ncores_from_cpuinfo();
uint32_t get_midr_from_cpuinfo(uint32_t core);
#endif

#endif
