#ifndef __UDEV_RISCV__
#define __UDEV_RISCV__

#include "../common/udev.h"

#define UNKNOWN -1

// https://elixir.bootlin.com/linux/v6.10.6/source/arch/riscv/include/asm/cpufeature.h#L21
struct riscv_cpuinfo {
  unsigned long mvendorid;
  unsigned long marchid;
  unsigned long mimpid;
};

char* get_hardware_from_devtree(void);
char* get_uarch_from_cpuinfo(void);
char* get_extensions_from_cpuinfo(void);
struct riscv_cpuinfo *get_riscv_cpuinfo(void);

#endif
