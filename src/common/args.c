#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "args.h"
#include "global.h"

#define COLOR_STR_INTEL "intel"
#define COLOR_STR_AMD   "amd"
#define COLOR_STR_ARM   "arm"

static const char *SYTLES_STR_LIST[] = {
  [STYLE_EMPTY]   = NULL,
  [STYLE_FANCY]   = "fancy",
  [STYLE_RETRO]   = "retro",
  [STYLE_LEGACY]  = "legacy",
  [STYLE_INVALID] = NULL
};

enum {
  ARG_CHAR_STYLE,
  ARG_CHAR_COLOR,
  ARG_CHAR_HELP,
#ifdef ARCH_X86
  ARG_CHAR_LEVELS,
#endif
  ARG_CHAR_VERBOSE,
  ARG_CHAR_VERSION
};

struct args_struct {
  bool levels_flag;
  bool help_flag;
  bool verbose_flag;
  bool version_flag;
  STYLE style;
  struct colors* colors;
};

static struct args_struct args;

STYLE get_style() {
  return args.style;
}

struct colors* get_colors() {
  return args.colors;
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
  return args.verbose_flag;
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

void free_colors_struct(struct colors* cs) {
  free(cs->c1);
  free(cs->c2);
  free(cs->c3);
  free(cs->c4);
  free(cs);
}

bool parse_color(char* optarg_str, struct colors** cs) {
  *cs = malloc(sizeof(struct colors));        
  (*cs)->c1 = malloc(sizeof(struct color));
  (*cs)->c2 = malloc(sizeof(struct color));
  (*cs)->c3 = malloc(sizeof(struct color));
  (*cs)->c4 = malloc(sizeof(struct color));
  struct color** c1 = &((*cs)->c1);
  struct color** c2 = &((*cs)->c2);
  struct color** c3 = &((*cs)->c3);
  struct color** c4 = &((*cs)->c4);
  int32_t ret;
  char* str_to_parse = NULL;
  bool free_ptr;
  
  if(strcmp(optarg_str, COLOR_STR_INTEL) == 0) {
    str_to_parse = malloc(sizeof(char) * 46);
    strcpy(str_to_parse, COLOR_DEFAULT_INTEL);
    free_ptr = true;
  }
  else if(strcmp(optarg_str, COLOR_STR_AMD) == 0) {
    str_to_parse = malloc(sizeof(char) * 44);
    strcpy(str_to_parse, COLOR_DEFAULT_AMD);  
    free_ptr = true;
  }
  else if(strcmp(optarg_str, COLOR_STR_ARM) == 0) {
    str_to_parse = malloc(sizeof(char) * 46);
    strcpy(str_to_parse, COLOR_DEFAULT_ARM);  
    free_ptr = true;
  }
  else {  
    str_to_parse = optarg_str;
    free_ptr = false;
  }
  
  ret = sscanf(str_to_parse, "%d,%d,%d:%d,%d,%d:%d,%d,%d:%d,%d,%d", 
               &(*c1)->R, &(*c1)->G, &(*c1)->B,
               &(*c2)->R, &(*c2)->G, &(*c2)->B,
               &(*c3)->R, &(*c3)->G, &(*c3)->B,
               &(*c4)->R, &(*c4)->G, &(*c4)->B);
  
  if(ret != 12) {
    printErr("Expected to read 12 values for color but read %d", ret);
    return false;    
  }
  
  //TODO: Refactor c1->R c2->R ... to c[i]->R
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
  
  if(free_ptr) free (str_to_parse);
  
  return true;      
}

bool parse_args(int argc, char* argv[]) {
  int c;
  int option_index = 0;  
  opterr = 0;

  bool color_flag = false;
  args.levels_flag = false;
  args.verbose_flag = false;
  args.help_flag = false;
  args.style = STYLE_EMPTY;
  args.colors = NULL;

  static struct option long_options[] = {
      {"style",    required_argument, 0, ARG_CHAR_STYLE   },
      {"color",    required_argument, 0, ARG_CHAR_COLOR   },
      {"help",     no_argument,       0, ARG_CHAR_HELP    },
#ifdef ARCH_X86
      {"levels",   no_argument,       0, ARG_CHAR_LEVELS  },
#endif
      {"verbose",  no_argument,       0, ARG_CHAR_VERBOSE },
      {"version",  no_argument,       0, ARG_CHAR_VERSION },
      {0, 0, 0, 0}
  };

  c = getopt_long(argc, argv, "", long_options, &option_index);

  while (c != -1) {
     if(c == ARG_CHAR_COLOR) {
       if(color_flag) {
         printErr("Color option specified more than once");
         return false;
       }
       color_flag  = true;       
       if(!parse_color(optarg, &args.colors)) {
         printErr("Color parsing failed");
         return false;
       }
     }
     else if(c == ARG_CHAR_STYLE) {
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
     else if(c == ARG_CHAR_HELP) {
       if(args.help_flag) {
         printErr("Help option specified more than once");
         return false;
       }
       args.help_flag  = true;
     }
     else if(c == ARG_CHAR_VERBOSE) {
       if(args.verbose_flag) {
         printErr("Verbose option specified more than once");
         return false;
       }
       args.verbose_flag  = true;
     }
#ifdef ARCH_X86
     else if(c == ARG_CHAR_LEVELS) {
       if(args.levels_flag) {
         printErr("Levels option specified more than once");
         return false;
       }
       args.levels_flag  = true;
     }
#endif
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
