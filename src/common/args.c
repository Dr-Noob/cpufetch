#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "args.h"
#include "global.h"

#define NUM_COLORS      5

#define COLOR_STR_INTEL     "intel"
#define COLOR_STR_INTEL_NEW "intel-new"
#define COLOR_STR_AMD       "amd"
#define COLOR_STR_IBM       "ibm"
#define COLOR_STR_ARM       "arm"
#define COLOR_STR_ROCKCHIP  "rockchip"
#define COLOR_STR_SIFIVE    "sifive"

static const char *SYTLES_STR_LIST[] = {
  [STYLE_EMPTY]   = NULL,
  [STYLE_FANCY]   = "fancy",
  [STYLE_RETRO]   = "retro",
  [STYLE_LEGACY]  = "legacy",
  [STYLE_INVALID] = NULL
};

struct args_struct {
  bool debug_flag;
  bool help_flag;
  bool raw_flag;
  bool accurate_pp;
  bool measure_max_frequency_flag;
  bool full_cpu_name_flag;
  bool logo_long;
  bool logo_short;
  bool logo_intel_new;
  bool logo_intel_old;
  bool verbose_flag;
  bool version_flag;
  STYLE style;
  struct color** colors;
};

const char args_chr[] = {
  /* [ARG_STYLE]            = */ 's',
  /* [ARG_COLOR]            = */ 'c',
  /* [ARG_HELP]             = */ 'h',
  /* [ARG_RAW]              = */ 'r',
  /* [ARG_FULLCPUNAME]      = */ 'F',
  /* [ARG_LOGO_LONG]        = */ 1,
  /* [ARG_LOGO_SHORT]       = */ 2,
  /* [ARG_LOGO_INTEL_NEW]   = */ 3,
  /* [ARG_LOGO_INTEL_OLD]   = */ 4,
  /* [ARG_ACCURATE_PP]      = */ 5,
  /* [ARG_MEASURE_MAX_FREQ] = */ 6,
  /* [ARG_DEBUG]            = */ 'd',
  /* [ARG_VERBOSE]          = */ 'v',
  /* [ARG_VERSION]          = */ 'V',
};

const char *args_str[] = {
  /* [ARG_STYLE]            = */ "style",
  /* [ARG_COLOR]            = */ "color",
  /* [ARG_HELP]             = */ "help",
  /* [ARG_RAW]              = */ "raw",
  /* [ARG_FULLCPUNAME]      = */ "full-cpu-name",
  /* [ARG_LOGO_LONG]        = */ "logo-long",
  /* [ARG_LOGO_SHORT]       = */ "logo-short",
  /* [ARG_LOGO_INTEL_NEW]   = */ "logo-intel-new",
  /* [ARG_LOGO_INTEL_OLD]   = */ "logo-intel-old",
  /* [ARG_ACCURATE_PP]      = */ "accurate-pp",
  /* [ARG_MEASURE_MAX_FREQ] = */ "measure-max-freq",
  /* [ARG_DEBUG]            = */ "debug",
  /* [ARG_VERBOSE]          = */ "verbose",
  /* [ARG_VERSION]          = */ "version",
};

static struct args_struct args;

STYLE get_style(void) {
  return args.style;
}

struct color** get_colors(void) {
  return args.colors;
}

bool show_help(void) {
  return args.help_flag;
}

bool show_version(void) {
  return args.version_flag;
}

bool show_debug(void) {
  return args.debug_flag;
}

bool show_raw(void) {
  return args.raw_flag;
}

bool accurate_pp(void) {
  return args.accurate_pp;
}

bool measure_max_frequency_flag(void) {
  return args.measure_max_frequency_flag;
}

bool show_full_cpu_name(void) {
  return args.full_cpu_name_flag;
}

bool show_logo_long(void) {
  return args.logo_long;
}

bool show_logo_short(void) {
  return args.logo_short;
}

bool show_logo_intel_new(void) {
  return args.logo_intel_new;
}

bool show_logo_intel_old(void) {
  return args.logo_intel_old;
}

bool verbose_enabled(void) {
  return args.verbose_flag;
}

int max_arg_str_length(void) {
  int max_len = -1;
  int len = sizeof(args_str) / sizeof(args_str[0]);
  for(int i=0; i < len; i++) {
    max_len = max(max_len, (int) strlen(args_str[i]));
  }
  return max_len;
}

STYLE parse_style(char* style) {
  uint8_t i = 0;
  uint8_t styles_count = sizeof(SYTLES_STR_LIST) / sizeof(SYTLES_STR_LIST[0]);

  while(i != styles_count && (SYTLES_STR_LIST[i] == NULL || strcmp(SYTLES_STR_LIST[i], style) != 0))
    i++;

  if(i == styles_count)
    return STYLE_INVALID;

  return i;
}

void free_colors_struct(struct color** cs) {
  for(int i=0; i < NUM_COLORS; i++) {
    free(cs[i]);
  }
  free(cs);
}

bool parse_color(char* optarg_str, struct color*** cs) {
  for(int i=0; i < NUM_COLORS; i++) {
    (*cs)[i] = emalloc(sizeof(struct color));
  }

  struct color** c = *cs;
  int32_t ret;
  char* str_to_parse = NULL;
  char* color_to_copy = NULL;
  bool free_ptr = true;

  if(strcmp(optarg_str, COLOR_STR_INTEL) == 0) color_to_copy = COLOR_DEFAULT_INTEL;
  else if(strcmp(optarg_str, COLOR_STR_INTEL_NEW) == 0) color_to_copy = COLOR_DEFAULT_INTEL_NEW;
  else if(strcmp(optarg_str, COLOR_STR_AMD) == 0) color_to_copy = COLOR_DEFAULT_AMD;
  else if(strcmp(optarg_str, COLOR_STR_IBM) == 0) color_to_copy = COLOR_DEFAULT_IBM;
  else if(strcmp(optarg_str, COLOR_STR_ARM) == 0) color_to_copy = COLOR_DEFAULT_ARM;
  else if(strcmp(optarg_str, COLOR_STR_ROCKCHIP) == 0) color_to_copy = COLOR_DEFAULT_ROCKCHIP;
  else if(strcmp(optarg_str, COLOR_STR_SIFIVE) == 0) color_to_copy = COLOR_DEFAULT_SIFIVE;
  else {
    str_to_parse = optarg_str;
    free_ptr = false;
  }

  if(str_to_parse == NULL) {
    str_to_parse = emalloc(sizeof(char) * (strlen(color_to_copy) + 1));
    strcpy(str_to_parse, color_to_copy);
  }

  ret = sscanf(str_to_parse, "%d,%d,%d:%d,%d,%d:%d,%d,%d:%d,%d,%d:%d,%d,%d",
               &c[0]->R, &c[0]->G, &c[0]->B,
               &c[1]->R, &c[1]->G, &c[1]->B,
               &c[2]->R, &c[2]->G, &c[2]->B,
               &c[3]->R, &c[3]->G, &c[3]->B,
               &c[4]->R, &c[4]->G, &c[4]->B);

  int expected_colors = 3 * NUM_COLORS;
  if(ret != expected_colors) {
    printErr("Expected to read %d values for color but read %d", expected_colors, ret);
    return false;
  }

  for(int i=0; i < NUM_COLORS; i++) {
    if(c[i]->R < 0 || c[i]->R > 255) {
      printErr("Red in color %d is invalid: %d; must be in range (0, 255)", i+1, c[i]->R);
      return false;
    }
    if(c[i]->G < 0 || c[i]->G > 255) {
      printErr("Green in color %d is invalid: %d; must be in range (0, 255)", i+1, c[i]->G);
      return false;
    }
    if(c[i]->B < 0 || c[i]->B > 255) {
      printErr("Blue in color %d is invalid: %d; must be in range (0, 255)", i+1, c[i]->B);
      return false;
    }
  }

  if(free_ptr) free (str_to_parse);

  return true;
}

char* build_short_options(void) {
  const char *c = args_chr;
  int len = sizeof(args_chr) / sizeof(args_chr[0]);
  char* str = (char *) ecalloc(len*2 + 1, sizeof(char));

#ifdef ARCH_X86
  sprintf(str, "%c:%c:%c%c%c%c%c%c%c%c%c%c%c%c",
  c[ARG_STYLE], c[ARG_COLOR], c[ARG_HELP],
  c[ARG_RAW], c[ARG_FULLCPUNAME],
  c[ARG_LOGO_SHORT], c[ARG_LOGO_LONG],
  c[ARG_LOGO_INTEL_NEW], c[ARG_LOGO_INTEL_OLD],
  c[ARG_ACCURATE_PP], c[ARG_MEASURE_MAX_FREQ],
  c[ARG_DEBUG], c[ARG_VERBOSE],
  c[ARG_VERSION]);
#elif ARCH_ARM
  sprintf(str, "%c:%c:%c%c%c%c%c%c%c",
  c[ARG_STYLE], c[ARG_COLOR], c[ARG_HELP],
  c[ARG_LOGO_SHORT], c[ARG_LOGO_LONG],
  c[ARG_MEASURE_MAX_FREQ],
  c[ARG_DEBUG], c[ARG_VERBOSE],
  c[ARG_VERSION]);
#else
  sprintf(str, "%c:%c:%c%c%c%c%c%c",
  c[ARG_STYLE], c[ARG_COLOR], c[ARG_HELP],
  c[ARG_LOGO_SHORT], c[ARG_LOGO_LONG],
  c[ARG_DEBUG], c[ARG_VERBOSE],
  c[ARG_VERSION]);
#endif

  return str;
}

bool parse_args(int argc, char* argv[]) {
  int opt;
  int option_index = 0;
  opterr = 0;

  bool color_flag = false;
  args.debug_flag = false;
  args.accurate_pp = false;
  args.full_cpu_name_flag = false;
  args.raw_flag = false;
  args.verbose_flag = false;
  args.logo_long = false;
  args.logo_short = false;
  args.logo_intel_new = false;
  args.logo_intel_old = false;
  args.help_flag = false;
  args.style = STYLE_EMPTY;
  args.colors = NULL;

  // Temporary enable verbose level to allow printing warnings inside parse_args
  set_log_level(true);

  const struct option long_options[] = {
    {args_str[ARG_STYLE],            required_argument, 0, args_chr[ARG_STYLE]            },
    {args_str[ARG_COLOR],            required_argument, 0, args_chr[ARG_COLOR]            },
    {args_str[ARG_HELP],             no_argument,       0, args_chr[ARG_HELP]             },
#ifdef ARCH_X86
    {args_str[ARG_LOGO_INTEL_NEW],   no_argument,       0, args_chr[ARG_LOGO_INTEL_NEW]   },
    {args_str[ARG_LOGO_INTEL_OLD],   no_argument,       0, args_chr[ARG_LOGO_INTEL_OLD]   },
    {args_str[ARG_ACCURATE_PP],      no_argument,       0, args_chr[ARG_ACCURATE_PP]      },
    {args_str[ARG_MEASURE_MAX_FREQ], no_argument,       0, args_chr[ARG_MEASURE_MAX_FREQ] },
    {args_str[ARG_FULLCPUNAME],      no_argument,       0, args_chr[ARG_FULLCPUNAME]      },
    {args_str[ARG_RAW],              no_argument,       0, args_chr[ARG_RAW]              },
#elif ARCH_ARM
    {args_str[ARG_MEASURE_MAX_FREQ], no_argument,       0, args_chr[ARG_MEASURE_MAX_FREQ] },
#endif
    {args_str[ARG_LOGO_SHORT],       no_argument,       0, args_chr[ARG_LOGO_SHORT]       },
    {args_str[ARG_LOGO_LONG],        no_argument,       0, args_chr[ARG_LOGO_LONG]        },
    {args_str[ARG_DEBUG],            no_argument,       0, args_chr[ARG_DEBUG]            },
    {args_str[ARG_VERBOSE],          no_argument,       0, args_chr[ARG_VERBOSE]          },
    {args_str[ARG_VERSION],          no_argument,       0, args_chr[ARG_VERSION]          },
    {0, 0, 0, 0}
  };

  char* short_options = build_short_options();
  opt = getopt_long(argc, argv, short_options, long_options, &option_index);

  while (!args.help_flag && !args.debug_flag && !args.version_flag && opt != -1) {
    if(opt == args_chr[ARG_COLOR]) {
      if(color_flag) {
        printErr("Color option specified more than once");
        return false;
      }
      color_flag  = true;
      args.colors = emalloc(sizeof(struct color *) * NUM_COLORS);
      if(!parse_color(optarg, &args.colors)) {
        return false;
      }
    }
    else if(opt == args_chr[ARG_STYLE]) {
      if(args.style != STYLE_EMPTY) {
        printErr("Style option specified more than once");
        return false;
      }
      args.style = parse_style(optarg);
      if(args.style == STYLE_INVALID) {
        printErr("Invalid style '%s'",optarg);
        return false;
      }
    }
    else if(opt == args_chr[ARG_HELP]) {
      args.help_flag  = true;
    }
    else if(opt == args_chr[ARG_ACCURATE_PP]) {
       args.accurate_pp = true;
    }
    else if(opt == args_chr[ARG_MEASURE_MAX_FREQ]) {
       args.measure_max_frequency_flag = true;
    }
    else if(opt == args_chr[ARG_FULLCPUNAME]) {
       args.full_cpu_name_flag = true;
    }
    else if(opt == args_chr[ARG_LOGO_SHORT]) {
       args.logo_short = true;
    }
    else if(opt == args_chr[ARG_LOGO_LONG]) {
       args.logo_long = true;
    }
    else if(opt == args_chr[ARG_LOGO_INTEL_NEW]) {
       args.logo_intel_new = true;
    }
    else if(opt == args_chr[ARG_LOGO_INTEL_OLD]) {
       args.logo_intel_old = true;
    }
    else if(opt == args_chr[ARG_RAW]) {
       args.raw_flag  = true;
    }
    else if(opt == args_chr[ARG_VERBOSE]) {
      args.verbose_flag  = true;
    }
    else if(opt == args_chr[ARG_DEBUG]) {
      args.debug_flag  = true;
    }
    else if(opt == args_chr[ARG_VERSION]) {
      args.version_flag = true;
    }
    else {
      printWarn("Invalid options");
      args.help_flag  = true;
    }

    option_index = 0;
    opt = getopt_long(argc, argv, short_options, long_options, &option_index);
  }

  if(optind < argc) {
    printWarn("Invalid options");
    args.help_flag  = true;
  }

  if(args.logo_intel_new && args.logo_intel_old) {
    printWarn("%s and %s cannot be specified together", args_str[ARG_LOGO_INTEL_NEW], args_str[ARG_LOGO_INTEL_OLD]);
    args.logo_intel_new = false;
    args.logo_intel_old = false;
  }

  if(args.logo_short && args.logo_long) {
    printWarn("%s and %s cannot be specified together", args_str[ARG_LOGO_SHORT], args_str[ARG_LOGO_LONG]);
    args.logo_short = false;
    args.logo_long = false;
  }

#if defined(ARCH_X86) && ! defined(__linux__)
  if(args.accurate_pp) {
    printWarn("%s option is valid only in Linux x86_64", args_str[ARG_ACCURATE_PP]);
    args.help_flag  = true;
  }
#endif

  // Leave log level untouched after returning
  set_log_level(false);

  return true;
}
