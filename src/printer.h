#ifndef __PRINTER__
#define __PRINTER__

typedef int STYLE;

#include "args.h"
#include "cpuid.h"

#define STYLES_COUNT 3

#define STYLE_INVALID -2
#define STYLE_EMPTY   -1
#define STYLE_FANCY    0
#define STYLE_RETRO    1
#define STYLE_LEGACY   2

bool print_cpufetch(struct cpuInfo* cpu, struct cache* cach, struct frequency* freq, struct topology* topo, STYLE s, struct colors* cs);

#endif
