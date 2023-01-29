#ifndef __ARGS__
#define __ARGS__

#include <stdbool.h>
#include <stdint.h>

struct color {
  int32_t R;
  int32_t G;
  int32_t B;
};

enum {
  STYLE_EMPTY,
  STYLE_FANCY,
  STYLE_RETRO,
  STYLE_LEGACY,
  STYLE_INVALID
};

enum {
  ARG_STYLE,
  ARG_COLOR,
  ARG_HELP,
  ARG_RAW,
  ARG_FULLCPUNAME,
  ARG_LOGO_LONG,
  ARG_LOGO_SHORT,
  ARG_LOGO_INTEL_NEW,
  ARG_LOGO_INTEL_OLD,
  ARG_ACCURATE_PP,
  ARG_DEBUG,
  ARG_VERBOSE,
  ARG_VERSION
};

extern const char args_chr[];
extern const char *args_str[];

#include "printer.h"

int max_arg_str_length(void);
bool parse_args(int argc, char* argv[]);
bool show_help(void);
bool accurate_pp(void);
bool show_full_cpu_name(void);
bool show_logo_long(void);
bool show_logo_short(void);
bool show_logo_intel_new(void);
bool show_logo_intel_old(void);
bool show_raw(void);
bool show_debug(void);
bool show_version(void);
bool verbose_enabled(void);
void free_colors_struct(struct color** cs);
struct color** get_colors(void);
STYLE get_style(void);

#endif
