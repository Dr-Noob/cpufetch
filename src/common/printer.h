#ifndef __PRINTER__
#define __PRINTER__

typedef int STYLE;

#include "args.h"

#ifdef ARCH_X86
  #include "../x86/cpuid.h"
#elif ARCH_PPC
  #include "../ppc/ppc.h"
#elif ARCH_ARM
  #include "../arm/midr.h"
#elif ARCH_RISCV
  #include "../riscv/riscv.h"
#endif

//                              +-----------------------------------+-----------------------+
//                              | Color logo                        | Color text            |
//                              | Color 1   | Color 2   | Color 3   | Color 1   | Color 2   |
#define COLOR_DEFAULT_INTEL     "015,125,194:230,230,230:000,000,000:040,150,220:230,230,230"
#define COLOR_DEFAULT_INTEL_NEW "030,204,251:250,250,250:000,104,181:230,230,230:030,204,251"
#define COLOR_DEFAULT_AMD       "250,250,250:000,154,102:000,000,000:250,250,250:000,154,102"
#define COLOR_DEFAULT_IBM       "092,119,172:092,119,172:000,000,000:240,240,240:092,119,172"
#define COLOR_DEFAULT_ARM       "000,145,189:000,145,189:000,000,000:240,240,240:000,145,189"
#define COLOR_DEFAULT_ROCKCHIP  "114,159,207:229,195,000:000,000,000:240,240,240:114,159,207"

#ifdef ARCH_X86
void print_levels(struct cpuInfo* cpu);
#endif

bool print_cpufetch(struct cpuInfo* cpu, STYLE s, struct color** cs, bool fcpuname);

#endif
