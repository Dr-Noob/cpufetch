#ifndef __ARGS__
#define __ARGS__

#include <stdbool.h>
#include <stdint.h>

struct color {
  int32_t R;
  int32_t G;
  int32_t B;
};

struct colors {
  struct color* c1;
  struct color* c2;
  struct color* c3;
  struct color* c4;
};

#include "printer.h"

bool parse_args(int argc, char* argv[]);
bool show_help();
bool show_levels();
bool show_version();
bool verbose_enabled();
void free_colors_struct(struct colors* cs);
struct colors* get_colors();
STYLE get_style();

#endif
