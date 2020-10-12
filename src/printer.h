#ifndef __PRINTER__
#define __PRINTER__

typedef int STYLE;

#include "args.h"
#include "cpuid.h"

#define COLOR_DEFAULT_INTEL "15,125,194:230,230,230:40,150,220:230,230,230"
#define COLOR_DEFAULT_AMD   "250,250,250:0,154,102:250,250,250:0,154,102"

bool print_cpufetch(struct cpuInfo* cpu, struct cache* cach, struct frequency* freq, struct topology* topo, STYLE s, struct colors* cs);

#endif
