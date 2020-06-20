#ifndef __ARGS__
#define __ARGS__

#include <stdbool.h>
#include "printer.h"

int parseArgs(int argc, char* argv[]);
STYLE getStyle();
int showHelp();
int showVersion();
bool verbose_enabled();

#endif
