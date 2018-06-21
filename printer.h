#ifndef __PRINTER__
#define __PRINTER__

#include "standart.h"
#include "ascii.h"

struct ascii;

struct ascii* set_ascii(VENDOR cpuVendor);
void print_ascii(struct ascii* art);
int setAttribute(struct ascii* art, int type, char* value);

#define BOOLEAN_TRUE 1
#define BOOLEAN_FALSE 0

#define ATTRIBUTE_COUNT 13
#define ATTRIBUTE_NAME      0
#define ATTRIBUTE_ARCH      1
#define ATTRIBUTE_FREQUENCY 2
#define ATTRIBUTE_NCORES    3
#define ATTRIBUTE_AVX       4
#define ATTRIBUTE_SSE       5
#define ATTRIBUTE_FMA       6
#define ATTRIBUTE_AES       7
#define ATTRIBUTE_SHA       8
#define ATTRIBUTE_L1        9
#define ATTRIBUTE_L2        10
#define ATTRIBUTE_L3        11
#define ATTRIBUTE_PEAK      12

#endif
