#ifndef __SOC__
#define __SOC__

#include "../common/cpu.h"
#include <stdint.h>

enum {
  SOC_UNKNOWN,
  SOC_SNAPDRAGON,
  SOC_MEDIATEK,
  SOC_EXYNOS,
};

struct system_on_chip {
  VENDOR soc_vendor;
  int32_t process;
  char* soc_name;
  char* raw_name;    
};

struct system_on_chip* get_soc();
char* get_soc_name(struct system_on_chip* soc);
VENDOR get_soc_vendor(struct system_on_chip* soc);
char* get_str_process(struct system_on_chip* soc);

#endif
