#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include "args.h"

#define ARG_STR_STYLE "style"
#define ARG_STR_HELP  "help"
#define ARG_CHAR_STYLE 's'
#define ARG_CHAR_HELP  'h'
#define STYLE_STR_1 "default"
#define STYLE_STR_2 "dark"

struct args_struct {
  int help_flag;
  STYLE style;
};

static const char* SYTLES_STR_LIST[STYLES_COUNT] = { STYLE_STR_1, STYLE_STR_2 };
static struct args_struct args;

STYLE parseStyle(char* style) {
  int i = 0;
  while(i != STYLES_COUNT && strcmp(SYTLES_STR_LIST[i],style) != 0)
    i++;

  if(i == STYLES_COUNT)
    return STYLE_INVALID;
  return i;
}

STYLE getStyle() {
  return args.style;
}

int showHelp() {
  return args.help_flag;
}

int parseArgs(int argc, char* argv[]) {
  int c;
  int digit_optind = 0;
  int option_index = 0;
  opterr = 0;

  args.help_flag = BOOLEAN_FALSE;
  args.style = STYLE_EMPTY;

  static struct option long_options[] = {
      {ARG_STR_STYLE, required_argument, 0, ARG_CHAR_STYLE},
      {ARG_STR_HELP,  no_argument,       0, ARG_CHAR_HELP },
      {0, 0, 0, 0}
  };

  c = getopt_long(argc, argv,"",long_options, &option_index);

  while (c != -1) {
     if(c == ARG_CHAR_STYLE) {
       if(args.style != STYLE_EMPTY) {
         printf("ERROR: Style option specified more than once\n");
         return BOOLEAN_FALSE;
       }
       args.style = parseStyle(optarg);
       if(args.style == STYLE_INVALID) {
          printf("ERROR: Invalid style '%s'\n",optarg);
          return BOOLEAN_FALSE;
       }
     }
     else if(c == ARG_CHAR_HELP) {
       if(args.help_flag) {
         printf("ERROR: Help option specified more than once\n");
         return BOOLEAN_FALSE;
       }
       args.help_flag  = BOOLEAN_TRUE;
     }
     else if(c == '?') {
       printf("WARNING: Invalid options\n");
       args.help_flag  = BOOLEAN_TRUE;
       break;
     }
     else
      printf("Bug at line number %d in file %s\n", __LINE__, __FILE__);

    option_index = 0;
    c = getopt_long(argc, argv,"",long_options, &option_index);
  }

  if (optind < argc) {
    printf("WARNING: Invalid options\n");
    args.help_flag  = BOOLEAN_TRUE;
  }

  return BOOLEAN_TRUE;
}
