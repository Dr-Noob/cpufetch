#ifndef __GLOBAL__
#define __GLOBAL__

#include <stdbool.h>
#include <stddef.h>

#define STRING_UNKNOWN "Unknown"

void set_log_level(bool verbose);
void printWarn(const char *fmt, ...);
void printErr(const char *fmt, ...);
void printBug(const char *fmt, ...);
int max(int a, int b);
void* emalloc(size_t size);
void* ecalloc(size_t nmemb, size_t size);

#endif
