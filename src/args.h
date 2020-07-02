#ifndef __ARGS__
#define __ARGS__

#include <stdbool.h>
#include <stdint.h>

bool parse_args(int argc, char* argv[]);
bool show_help();
bool show_levels();
bool show_version();
bool verbose_enabled();
struct color* get_color1();
struct color* get_color2();

struct color {
  int32_t R;
  int32_t G;
  int32_t B;
};

#include "printer.h"

#endif
