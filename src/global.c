#include <stdarg.h>
#include <stdio.h>
#include "global.h"

#define RED "\x1b[31;1m"
#define RESET "\x1b[0m"

void printErr(const char *fmt, ...) {
  int buffer_size = 4096;
  char buffer[buffer_size];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer,buffer_size, fmt, args);
  va_end(args);
  fprintf(stderr,RED "ERROR: "RESET "%s\n",buffer);
}

void printBug(const char *fmt, ...) {
  int buffer_size = 4096;
  char buffer[buffer_size];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer,buffer_size, fmt, args);
  va_end(args);
  fprintf(stderr,RED "ERROR: "RESET "%s\n",buffer);
  fprintf(stderr,"Please, create a new issue with this error message and your CPU in https://github.com/Dr-Noob/cpufetch/issues\n");
}
