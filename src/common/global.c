#include <stdarg.h>
#include <stdio.h>
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
#ifdef ARCH_X86
  fprintf(stderr,"Please, create a new issue with this error message and the output of 'cpufetch --debug' in https://github.com/Dr-Noob/cpufetch/issues\n");
#elif ARCH_ARM
  fprintf(stderr,"Please, create a new issue with this error message, your smartphone/computer model and the output of 'cpufetch --debug' in https://github.com/Dr-Noob/cpufetch/issues\n");
#endif
}

void set_log_level(bool verbose) {
  if(verbose) LOG_LEVEL = LOG_LEVEL_VERBOSE;
  else LOG_LEVEL = LOG_LEVEL_NORMAL;
}
