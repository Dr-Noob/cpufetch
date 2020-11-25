#ifndef __SOC__
#define __SOC__

#include <stdint.h>

typedef int32_t SOC;

enum {
  SOC_UNKNOWN,
  SOC_SNAPDRAGON,
  SOC_MEDIATEK,
  SOC_EXYNOS,
};

struct system_on_chip {
  SOC soc_vendor;
  int32_t process;
  char* soc_name;
  char* raw_name;    
};

struct system_on_chip* get_soc();
char* get_soc_name(struct system_on_chip* soc);

#endif
