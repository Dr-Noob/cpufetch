#ifndef __UDEV_ARM__
#define __UDEV_ARM__

#include "../common/udev.h"

#define _PATH_SUNXI_NVMEM  "/sys/bus/nvmem/devices/sunxi-sid0/nvmem"
#define _PATH_RK_EFUSE0    "/sys/bus/nvmem/devices/rockchip-efuse0/nvmem"

#define UNKNOWN -1
int get_ncores_from_cpuinfo(void);
uint32_t get_midr_from_cpuinfo(uint32_t core, bool* success);
char* get_hardware_from_cpuinfo(void);
char* get_revision_from_cpuinfo(void);
bool is_raspberry_pi(void);

#endif

