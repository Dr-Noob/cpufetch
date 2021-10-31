#ifndef __UDEV_ARM__
#define __UDEV_ARM__

#include "../common/udev.h"

#define UNKNOWN -1
int get_ncores_from_cpuinfo();
uint32_t get_midr_from_cpuinfo(uint32_t core, bool* success);
char* get_hardware_from_cpuinfo();
char* get_revision_from_cpuinfo();
bool is_raspberry_pi();

#endif

