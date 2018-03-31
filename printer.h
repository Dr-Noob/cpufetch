#ifndef __PRINTER__
#define __PRINTER__

#include "standart.h"
#include "ascii.h"

struct ascii;

struct ascii* set_ascii(VENDOR cpuVendor);
void print_ascii(struct ascii* art, int n);
void print_text(struct ascii* art,char* title, char* text);

#define BOOLEAN_TRUE 1
#define BOOLEAN_FALSE 0

#define TITLE_NAME      "Name:       "
#define TITLE_ARCH      "Arch:       "
#define TITLE_FREQUENCY "Frequency:  "
#define TITLE_NCORES    "N.Cores:    "
#define TITLE_AVX       "AVX:        "
#define TITLE_SSE       "SSE:        "
#define TITLE_FMA       "FMA:        "
#define TITLE_AES       "AES:        "
#define TITLE_SHA       "SHA:        "
#define TITLE_L1        "L1 Size:    "
#define TITLE_L2        "L2 Size:    "
#define TITLE_L3        "L3 Size:    "
#define TITLE_PEAK      "Peak FLOPS: "

#endif
