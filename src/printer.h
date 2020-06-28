#ifndef __PRINTER__
#define __PRINTER__

#include "cpuid.h"

typedef int STYLE;
#define STYLES_COUNT 3

#define STYLE_EMPTY   -2
#define STYLE_INVALID -1
#define STYLE_DEFAULT  0
#define STYLE_DARK     1
#define STYLE_NONE     2

bool print_cpufetch(struct cpuInfo* cpu, struct cache* cach, struct frequency* freq, struct topology* topo, STYLE s);

#endif
