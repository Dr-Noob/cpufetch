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

#define _PATH_FREQUENCY_MAX_LEN 100
#define DEFAULT_FILE_SIZE       4096

char* read_file(char* path, int* len);
long get_max_freq_from_file(uint32_t core, bool hv_present);
long get_min_freq_from_file(uint32_t core, bool hv_present);

#endif
