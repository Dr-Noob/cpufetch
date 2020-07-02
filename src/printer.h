#ifndef __PRINTER__
#define __PRINTER__

#include "args.h"
#include "cpuid.h"

bool print_cpufetch(struct cpuInfo* cpu, struct cache* cach, struct frequency* freq, struct topology* topo, struct color* c1, struct color* c2);

#endif
