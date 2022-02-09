#ifndef __UDEV__
#define __UDEV__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "cpu.h"

#define _PATH_SYS_SYSTEM        "/sys/devices/system"
#define _PATH_SYS_CPU           "/cpu"
#define _PATH_FREQUENCY         "/cpufreq"
#define _PATH_FREQUENCY_MAX     "/cpuinfo_max_freq"
#define _PATH_FREQUENCY_MIN     "/cpuinfo_min_freq"
#define _PATH_CACHE_L1D         "/cache/index0"
#define _PATH_CACHE_L1I         "/cache/index1"
#define _PATH_CACHE_L2          "/cache/index2"
#define _PATH_CACHE_L3          "/cache/index3"
#define _PATH_CACHE_SIZE        "/size"
#define _PATH_CACHE_SHARED_MAP  "/shared_cpu_map"
#define _PATH_CPUS_PRESENT           _PATH_SYS_SYSTEM _PATH_SYS_CPU "/present"

#define _PATH_FREQUENCY_MAX_LEN 100
#define _PATH_CACHE_MAX_LEN     200

char* read_file(char* path, int* len);
long get_max_freq_from_file(uint32_t core);
long get_min_freq_from_file(uint32_t core);
long get_l1i_cache_size(uint32_t core);
long get_l1d_cache_size(uint32_t core);
long get_l2_cache_size(uint32_t core);
long get_l3_cache_size(uint32_t core);
int get_num_caches_by_level(struct cpuInfo* cpu, uint32_t level);
int get_ncores_from_cpuinfo();

#endif
