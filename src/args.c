#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "args.h"
#include "global.h"

#define ARG_STR_COLOR    "color"
#define ARG_STR_HELP     "help"
#define ARG_STR_LEVELS   "levels"
#define ARG_STR_VERSION  "version"
#define ARG_CHAR_COLOR    'c'
#define ARG_CHAR_HELP     'h'
#define ARG_CHAR_LEVELS   'l'
#define ARG_CHAR_VERSION  'v'

struct args_struct {
  bool levels_flag;
  bool help_flag;
  bool version_flag;
  struct color* color1;
  struct color* color2;
};

static struct args_struct args;

struct color* get_color1() {
  return args.color1;
}

struct color* get_color2() {
  return args.color2;
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

bool parse_color(char* optarg, struct color** c1, struct color** c2) {
  *c1 = malloc(sizeof(struct color));
  *c2 = malloc(sizeof(struct color));
  int32_t ret;
  
  ret = sscanf(optarg, "%d,%d,%d:%d,%d,%d", &(*c1)->R, &(*c1)->G, &(*c1)->B, &(*c2)->R, &(*c2)->G, &(*c2)->B);
  
  if(ret != 6) {
    printErr("Expected to read 6 values for color 1 but read %d", ret);
    return false;    
  }
  
  if((*c1)->R < 0 || (*c1)->R > 255) {
    printErr("Red in color 1 is invalid. Must be in range (0, 255)");
    return false;
  }
  if((*c1)->G < 0 || (*c1)->G > 255) {
    printErr("Green in color 1 is invalid. Must be in range (0, 255)");
    return false;
  }
  if((*c1)->B < 0 || (*c1)->B > 255) {
    printErr("Blue in color 1 is invalid. Must be in range (0, 255)");
    return false;
  }
  if((*c2)->R < 0 || (*c2)->R > 255) {
    printErr("Red in color 2 is invalid. Must be in range (0, 255)");
    return false;
  }
  if((*c2)->G < 0 || (*c2)->G > 255) {
    printErr("Green in color 2 is invalid. Must be in range (0, 255)");
    return false;
  }
  if((*c2)->B < 0 || (*c2)->B > 255) {
    printErr("Blue in color 2 is invalid. Must be in range (0, 255)");
    return false;
  }
  
  return true;      
}

bool parse_args(int argc, char* argv[]) {
  int c;
  int digit_optind = 0;
  int option_index = 0;  
  opterr = 0;

  bool color_flag = false;
  args.levels_flag = false;
  args.help_flag = false;

  static struct option long_options[] = {
      {ARG_STR_COLOR,    required_argument, 0, ARG_CHAR_COLOR   },
      {ARG_STR_HELP,     no_argument,       0, ARG_CHAR_HELP    },
      {ARG_STR_LEVELS,   no_argument,       0, ARG_CHAR_LEVELS  },
      {ARG_STR_VERSION,  no_argument,       0, ARG_CHAR_VERSION },
      {0, 0, 0, 0}
  };

  c = getopt_long(argc, argv,"",long_options, &option_index);

  while (c != -1) {
     if(c == ARG_CHAR_COLOR) {
       if(color_flag) {
         printErr("Color option specified more than once");
         return false;
       }
       color_flag  = true;       
       if(!parse_color(optarg, &args.color1, &args.color2)) {
         printErr("Color parsing failed");
         return false;
       }
     }
     else if(c == ARG_CHAR_HELP) {
       if(args.help_flag) {
         printErr("Help option specified more than once");
         return false;
       }
       args.help_flag  = true;
     }
     else if(c == ARG_CHAR_LEVELS) {
       if(args.levels_flag) {
         printErr("Levels option specified more than once");
         return false;
       }
       args.levels_flag  = true;
     }
     else if (c == ARG_CHAR_VERSION) {
       if(args.version_flag) {
         printErr("Version option specified more than once");
         return false;
       }
       args.version_flag = true;
     }
     else if(c == '?') {
       printWarn("Invalid options");
       args.help_flag  = true;
       break;
     }
     else
      printBug("Bug at line number %d in file %s", __LINE__, __FILE__);

    option_index = 0;
    c = getopt_long(argc, argv,"",long_options, &option_index);
  }

  if (optind < argc) {
    printWarn("Invalid options");
    args.help_flag  = true;
  }

  if((args.help_flag + args.version_flag + color_flag) > 1) {
    printWarn("You should specify just one option");
    args.help_flag  = true;
  }

  return true;
}
