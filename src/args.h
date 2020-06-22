#ifndef __ARGS__
#define __ARGS__

#include <stdbool.h>
#include "printer.h"

bool parse_args(int argc, char* argv[]);
STYLE get_style();
bool show_help();
bool show_levels();
bool show_version();
bool verbose_enabled();

#endif
