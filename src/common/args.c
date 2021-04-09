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

struct args_struct {
  bool debug_flag;
  bool help_flag;
  bool raw_flag;
  bool verbose_flag;
  bool version_flag;
  STYLE style;
  struct colors* colors;
};

const char args_chr[] = {
  /* [ARG_CHAR_STYLE]   = */ 's',
  /* [ARG_CHAR_COLOR]   = */ 'c',
  /* [ARG_CHAR_HELP]    = */ 'h',
  /* [ARG_CHAR_RAW]     = */ 'r',
  /* [ARG_CHAR_DEBUG]   = */ 'd',
  /* [ARG_CHAR_VERBOSE] = */ 'v',
  /* [ARG_CHAR_VERSION] = */ 'V',
};

const char *args_str[] = {
  /* [ARG_CHAR_STYLE]   = */ "style",
  /* [ARG_CHAR_COLOR]   = */ "color",
  /* [ARG_CHAR_HELP]    = */ "help",
  /* [ARG_CHAR_RAW]     = */ "raw",
  /* [ARG_CHAR_DEBUG]   = */ "debug",
  /* [ARG_CHAR_VERBOSE] = */ "verbose",
  /* [ARG_CHAR_VERSION] = */ "version",
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

bool show_debug() {
  return args.debug_flag;
}

bool show_raw() {
  return args.raw_flag;    
}

bool verbose_enabled() {
  return args.verbose_flag;
}

int max_arg_str_length() {
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

char* build_short_options() {
  const char *c = args_chr;
  int len = sizeof(args_chr) / sizeof(args_chr[0]);
  char* str = (char *) malloc(sizeof(char) * (len*2 + 1));
  memset(str, 0, sizeof(char) * (len*2 + 1));

#ifdef ARCH_X86
  sprintf(str, "%c%c:%c:%c%c%c%c",
  c[ARG_RAW],
  c[ARG_STYLE], c[ARG_COLOR], c[ARG_HELP],
  c[ARG_DEBUG], c[ARG_VERBOSE], c[ARG_VERSION]);
#else
  sprintf(str, "%c:%c:%c%c%c%c",
  c[ARG_STYLE], c[ARG_COLOR], c[ARG_HELP],
  c[ARG_DEBUG], c[ARG_VERBOSE], c[ARG_VERSION]);
#endif

  return str;
}

bool parse_args(int argc, char* argv[]) {
  int opt;
  int option_index = 0;  
  opterr = 0;

  bool color_flag = false;
  args.debug_flag = false;
  args.raw_flag = false;
  args.verbose_flag = false;
  args.help_flag = false;
  args.style = STYLE_EMPTY;
  args.colors = NULL;

  const struct option long_options[] = {
    {args_str[ARG_STYLE],   required_argument, 0, args_chr[ARG_STYLE]   },
    {args_str[ARG_COLOR],   required_argument, 0, args_chr[ARG_COLOR]   },
    {args_str[ARG_HELP],    no_argument,       0, args_chr[ARG_HELP]    },
#ifdef ARCH_X86
    {args_str[ARG_RAW],     no_argument,       0, args_chr[ARG_RAW]     },
#endif
    {args_str[ARG_DEBUG],   no_argument,       0, args_chr[ARG_DEBUG]   },
    {args_str[ARG_VERBOSE], no_argument,       0, args_chr[ARG_VERBOSE] },
    {args_str[ARG_VERSION], no_argument,       0, args_chr[ARG_VERSION] },
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
      if(!parse_color(optarg, &args.colors)) {
        printErr("Color parsing failed");
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
      break;
    }
    else if(opt == args_chr[ARG_HELP]) {
      args.help_flag  = true;
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

  if((args.help_flag + args.version_flag + color_flag) > 1) {
    printWarn("You should specify just one option");
    args.help_flag  = true;
  }

  return true;
}
