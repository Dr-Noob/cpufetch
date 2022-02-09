#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "global.h"

#ifdef _WIN32

#define RED   ""
#define BOLD  ""
#define RESET ""

#else

#define RED "\x1b[31;1m"
#define BOLD "\x1b[;1m"
#define RESET "\x1b[0m"

#endif

enum {
  LOG_LEVEL_NORMAL,
  LOG_LEVEL_VERBOSE
};

int LOG_LEVEL;

void printWarn(const char *fmt, ...) {
  if(LOG_LEVEL == LOG_LEVEL_VERBOSE) {
    int buffer_size = 4096;
    char buffer[buffer_size];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer,buffer_size, fmt, args);
    va_end(args);
    fprintf(stderr,BOLD "[WARNING]: "RESET "%s\n",buffer);
  }
}

void printErr(const char *fmt, ...) {
  int buffer_size = 4096;
  char buffer[buffer_size];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer,buffer_size, fmt, args);
  va_end(args);
  fprintf(stderr,RED "[ERROR]: "RESET "%s\n",buffer);
}

void printBug(const char *fmt, ...) {
  int buffer_size = 4096;
  char buffer[buffer_size];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer,buffer_size, fmt, args);
  va_end(args);
  fprintf(stderr,RED "[ERROR]: "RESET "%s\n",buffer);
#if defined(ARCH_X86) || defined(ARCH_PPC)
  fprintf(stderr, "Please, create a new issue with this error message, the output of 'cpufetch' and 'cpufetch --debug' on https://github.com/Dr-Noob/cpufetch/issues\n");
#elif ARCH_ARM
  fprintf(stderr, "Please, create a new issue with this error message, your smartphone/computer model, the output of 'cpufetch' and 'cpufetch --debug' on https://github.com/Dr-Noob/cpufetch/issues\n");
#endif
}

void set_log_level(bool verbose) {
  if(verbose) LOG_LEVEL = LOG_LEVEL_VERBOSE;
  else LOG_LEVEL = LOG_LEVEL_NORMAL;
}

int max(int a, int b) {
  return a > b ? a : b;
}

int min(int a, int b) {
  return a < b ? a : b;
}

char *strremove(char *str, const char *sub) {
  char *p, *q, *r;
  if (*sub && (q = r = strstr(str, sub)) != NULL) {
    size_t len = strlen(sub);
    while ((r = strstr(p = r + len, sub)) != NULL) {
      memmove(q, p, r - p);
      q += r - p;
    }
    memmove(q, p, strlen(p) + 1);
  }
  return str;
}

void* emalloc(size_t size) {
  void* ptr = malloc(size);

  if(ptr == NULL) {
    printErr("malloc failed: %s", strerror(errno));
    exit(1);
  }

  return ptr;
}

void* ecalloc(size_t nmemb, size_t size) {
  void* ptr = calloc(nmemb, size);

  if(ptr == NULL) {
    printErr("calloc failed: %s", strerror(errno));
    exit(1);
  }

  return ptr;
}

void* erealloc(void *ptr, size_t size) {
  void* newptr = realloc(ptr, size);

  if(newptr == NULL) {
    printErr("realloc failed: %s", strerror(errno));
    exit(1);
  }

  return newptr;
}

