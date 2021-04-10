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

enum {
  STYLE_EMPTY,
  STYLE_FANCY,
  STYLE_WILD,
  STYLE_RETRO,
  STYLE_LEGACY,
  STYLE_INVALID
};

enum {
  ARG_STYLE,
  ARG_COLOR,
  ARG_LOOP,
  ARG_HELP,
  ARG_RAW,
  ARG_DEBUG,
  ARG_VERBOSE,
  ARG_VERSION
};

extern const char args_chr[];
extern const char *args_str[];

#include "printer.h"

int max_arg_str_length();
bool parse_args(int argc, char* argv[]);
bool show_help();
bool loop_mode();
bool show_raw();
bool show_debug();
bool show_version();
bool verbose_enabled();
void free_colors_struct(struct colors* cs);
struct colors* get_colors();
STYLE get_style();

#endif
