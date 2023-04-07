#ifndef __UDEV_RISCV__
#define __UDEV_RISCV__

#include "../common/udev.h"

#define UNKNOWN -1

char* get_hardware_from_devtree(void);
char* get_uarch_from_cpuinfo(void);

#endif
