#ifndef __PRINTER__
#define __PRINTER__

typedef int STYLE;

#include "args.h"

#ifdef ARCH_X86
  #include "../x86/cpuid.h"
#else
  #include "../arm/midr.h"
#endif

#define COLOR_DEFAULT_INTEL "15,125,194:230,230,230:40,150,220:230,230,230"
#define COLOR_DEFAULT_AMD   "250,250,250:0,154,102:250,250,250:0,154,102"
#define COLOR_DEFAULT_ARM   "0,145,189:0,145,189:240,240,240:0,145,189"

#ifdef ARCH_X86
void print_levels(struct cpuInfo* cpu);
#endif

bool print_cpufetch(struct cpuInfo* cpu, STYLE s, struct colors* cs);

#endif
