#ifndef __PRINTER__
#define __PRINTER__

#include "standart.h"
#include "ascii.h"

#define BOOLEAN_TRUE 1
#define BOOLEAN_FALSE 0

#define ATTRIBUTE_COUNT 12
#define ATTRIBUTE_NAME      0
#define ATTRIBUTE_FREQUENCY 1
#define ATTRIBUTE_NCORES    2
#define ATTRIBUTE_AVX       3
#define ATTRIBUTE_SSE       4
#define ATTRIBUTE_FMA       5
#define ATTRIBUTE_AES       6
#define ATTRIBUTE_SHA       7
#define ATTRIBUTE_L1        8
#define ATTRIBUTE_L2        9
#define ATTRIBUTE_L3        10
#define ATTRIBUTE_PEAK      11

typedef int STYLE;
#define STYLES_COUNT 2

#define STYLE_EMPTY -2
#define STYLE_INVALID -1
#define STYLE_DEFAULT 0
#define STYLE_DARK 1

struct ascii;

static const int STYLES_CODE_LIST [STYLES_COUNT] = {STYLE_DEFAULT, STYLE_DARK};
struct ascii* set_ascii(VENDOR cpuVendor, STYLE style);
void print_ascii(struct ascii* art);
int setAttribute(struct ascii* art, int type, char* value);

#endif
