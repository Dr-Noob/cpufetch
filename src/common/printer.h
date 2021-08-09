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
#endif

#define COLOR_DEFAULT_INTEL "15,125,194:230,230,230:40,150,220:230,230,230"
#define COLOR_DEFAULT_AMD   "250,250,250:0,154,102:250,250,250:0,154,102"
#define COLOR_DEFAULT_IBM   "92,119,172:92,119,172:240,240,240:92,119,172"
#define COLOR_DEFAULT_ARM   "0,145,189:0,145,189:240,240,240:0,145,189"
/* #define COLOR_DEFAULT_M1    "255,250,250:255,250,250:255,250,250:255,250,250" */

#ifdef ARCH_X86
void print_levels(struct cpuInfo* cpu);
#endif

bool print_cpufetch(struct cpuInfo* cpu, STYLE s, struct color** cs);

#endif
