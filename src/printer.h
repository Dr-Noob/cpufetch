#ifndef __PRINTER__
#define __PRINTER__

typedef int STYLE;

#include "args.h"
#include "cpuid.h"

#define STYLES_COUNT 2

#define STYLE_INVALID -2
#define STYLE_EMPTY   -1
#define STYLE_FANCY    0
#define STYLE_RETRO    1

bool print_cpufetch(struct cpuInfo* cpu, struct cache* cach, struct frequency* freq, struct topology* topo, STYLE s, struct color* c1, struct color* c2);

#endif
