#ifndef __GLOBAL__
#define __GLOBAL__

#include <stdbool.h>

void set_log_level(bool verbose);
void printWarn(const char *fmt, ...);
void printErr(const char *fmt, ...);
void printBug(const char *fmt, ...);
int max(int a, int b);

#endif
