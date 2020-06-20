#include <stdarg.h>
#include <stdio.h>
#include "global.h"

#define RED "\x1b[31;1m"
#define BOLD "\x1b[;1m"
#define RESET "\x1b[0m"

#define LOG_LEVEL_NORMAL  0
#define LOG_LEVEL_VERBOSE 1

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
  fprintf(stderr,"Please, create a new issue with this error message and your CPU in https://github.com/Dr-Noob/cpufetch/issues\n");
}

void set_log_level(bool verbose) {
  if(verbose) LOG_LEVEL = LOG_LEVEL_VERBOSE;
  else LOG_LEVEL = LOG_LEVEL_NORMAL;
}
