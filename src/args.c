#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include "args.h"

#define ARG_STR_STYLE    "style"
#define ARG_STR_HELP     "help"
#define ARG_STR_LEVELS   "levels"
#define ARG_STR_VERSION  "version"
#define ARG_CHAR_STYLE    's'
#define ARG_CHAR_HELP     'h'
#define ARG_CHAR_LEVELS   'l'
#define ARG_CHAR_VERSION  'v'
#define STYLE_STR_1 "default"
#define STYLE_STR_2 "dark"
#define STYLE_STR_3 "none"

struct args_struct {
  bool levels_flag;
  bool help_flag;
  bool version_flag;
  STYLE style;
};

static const char* SYTLES_STR_LIST[STYLES_COUNT] = { STYLE_STR_1, STYLE_STR_2, STYLE_STR_3 };
static struct args_struct args;

STYLE parse_style(char* style) {
  int i = 0;
  while(i != STYLES_COUNT && strcmp(SYTLES_STR_LIST[i],style) != 0)
    i++;

  if(i == STYLES_COUNT)
    return STYLE_INVALID;
  return i;
}

STYLE get_style() {
  return args.style;
}

bool show_help() {
  return args.help_flag;
}

bool show_version() {
  return args.version_flag;
}

bool show_levels() {
  return args.levels_flag;
}

bool verbose_enabled() {
  return false;
}

bool parse_args(int argc, char* argv[]) {
  int c;
  int digit_optind = 0;
  int option_index = 0;
  opterr = 0;

  args.levels_flag = false;
  args.help_flag = false;
  args.style = STYLE_EMPTY;

  static struct option long_options[] = {
      {ARG_STR_STYLE,    required_argument, 0, ARG_CHAR_STYLE   },
      {ARG_STR_HELP,     no_argument,       0, ARG_CHAR_HELP    },
      {ARG_STR_LEVELS,   no_argument,       0, ARG_CHAR_LEVELS  },
      {ARG_STR_VERSION,  no_argument,       0, ARG_CHAR_VERSION },
      {0, 0, 0, 0}
  };

  c = getopt_long(argc, argv,"",long_options, &option_index);

  while (c != -1) {
     if(c == ARG_CHAR_STYLE) {
       if(args.style != STYLE_EMPTY) {
         printf("ERROR: Style option specified more than once\n");
         return false;
       }
       args.style = parse_style(optarg);
       if(args.style == STYLE_INVALID) {
          printf("ERROR: Invalid style '%s'\n",optarg);
          return false;
       }
     }
     else if(c == ARG_CHAR_HELP) {
       if(args.help_flag) {
         printf("ERROR: Help option specified more than once\n");
         return false;
       }
       args.help_flag  = true;
     }
     else if(c == ARG_CHAR_LEVELS) {
       if(args.levels_flag) {
         printf("ERROR: Levels option specified more than once\n");
         return false;
       }
       args.levels_flag  = true;
     }
     else if (c == ARG_CHAR_VERSION) {
       if(args.version_flag) {
         printf("ERROR: Version option specified more than once\n");
         return false;
       }
       args.version_flag = true;
     }
     else if(c == '?') {
       printf("WARNING: Invalid options\n");
       args.help_flag  = true;
       break;
     }
     else
      printf("Bug at line number %d in file %s\n", __LINE__, __FILE__);

    option_index = 0;
    c = getopt_long(argc, argv,"",long_options, &option_index);
  }

  if (optind < argc) {
    printf("WARNING: Invalid options\n");
    args.help_flag  = true;
  }

  if((args.help_flag + args.version_flag + (args.style != STYLE_EMPTY)) > 1) {
    printf("WARNING: You should specify just one option\n");
    args.help_flag  = true;
  }

  return true;
}
