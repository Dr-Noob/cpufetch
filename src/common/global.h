#ifndef __GLOBAL__
#define __GLOBAL__

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#define STRING_UNKNOWN "Unknown"
#define UNUSED(x) (void)(x)

void set_log_level(bool verbose);
void printWarn(const char *fmt, ...);
void printErr(const char *fmt, ...);
void printBug(const char *fmt, ...);
void printBugCheckRelease(const char *fmt, ...);
int min(int a, int b);
int max(int a, int b);
char *strremove(char *str, const char *sub);
void* emalloc(size_t size);
void* ecalloc(size_t nmemb, size_t size);
void* erealloc(void *ptr, size_t size);
#ifndef __APPLE__
bool bind_to_cpu(int cpu_id);
#endif
void print_version(FILE *restrict stream);

#endif
