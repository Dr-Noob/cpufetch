#ifndef __PRINTER__
#define __PRINTER__

#include "standart.h"
#include "ascii.h"

#define ATTRIBUTE_COUNT    13
#define ATTRIBUTE_NAME      0
#define ATTRIBUTE_FREQUENCY 1
#define ATTRIBUTE_NCORES    2
#define ATTRIBUTE_AVX       3
#define ATTRIBUTE_SSE       4
#define ATTRIBUTE_FMA       5
#define ATTRIBUTE_AES       6
#define ATTRIBUTE_SHA       7
#define ATTRIBUTE_L1i       8
#define ATTRIBUTE_L1d       9
#define ATTRIBUTE_L2        10
#define ATTRIBUTE_L3        11
#define ATTRIBUTE_PEAK      12

typedef int STYLE;
#define STYLES_COUNT 3

#define STYLE_EMPTY   -2
#define STYLE_INVALID -1
#define STYLE_DEFAULT  0
#define STYLE_DARK     1
#define STYLE_NONE     2

struct ascii;

static const int STYLES_CODE_LIST [STYLES_COUNT] = {STYLE_DEFAULT, STYLE_DARK};
struct ascii* set_ascii(VENDOR cpuVendor, STYLE style);
void print_ascii(struct ascii* art);
void setAttribute(struct ascii* art, int type, char* value);

#endif
