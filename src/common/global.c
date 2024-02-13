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

#ifdef ARCH_X86
  static const char* ARCH_STR = "x86 / x86_64 build";
  #include "../x86/cpuid.h"
#elif ARCH_PPC
  static const char* ARCH_STR = "PowerPC build";
  #include "../ppc/ppc.h"
#elif ARCH_ARM
  static const char* ARCH_STR = "ARM build";
  #include "../arm/midr.h"
#elif ARCH_RISCV
  static const char* ARCH_STR = "RISC-V build";
  #include "../riscv/riscv.h"
#endif

#ifdef __linux__
  #ifdef __ANDROID__
    static const char* OS_STR = "Android";
  #else
    static const char* OS_STR = "Linux";
  #endif
#elif __FreeBSD__
  static const char* OS_STR = "FreeBSD";
#elif _WIN32
  static const char* OS_STR = "Windows";
#elif defined __APPLE__ || __MACH__
  static const char* OS_STR = "macOS";
#else
  static const char* OS_STR = "Unknown OS";
#endif

#ifndef GIT_FULL_VERSION
  static const char* VERSION = "1.05";
#endif

enum {
  LOG_LEVEL_NORMAL,
  LOG_LEVEL_VERBOSE
};

int LOG_LEVEL;

void printBugMessage(FILE *restrict stream) {
  #if defined(ARCH_X86) || defined(ARCH_PPC)
    fprintf(stream, "Please, create a new issue with this error message, the output of 'cpufetch' and 'cpufetch --debug' on https://github.com/Dr-Noob/cpufetch/issues\n");
  #elif ARCH_ARM
    fprintf(stream, "Please, create a new issue with this error message, your smartphone/computer model, the output of 'cpufetch --verbose' and 'cpufetch --debug' on https://github.com/Dr-Noob/cpufetch/issues\n");
  #endif
}

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
  fprintf(stderr,"[VERSION]: ");
  print_version(stderr);
}

void printBug(const char *fmt, ...) {
  int buffer_size = 4096;
  char buffer[buffer_size];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer,buffer_size, fmt, args);
  va_end(args);
  fprintf(stderr,RED "[ERROR]: "RESET "%s\n",buffer);
  fprintf(stderr,"[VERSION]: ");
  print_version(stderr);
  printBugMessage(stderr);
}

bool isReleaseVersion(char *git_full_version) {
  return strstr(git_full_version, "-") == NULL;
}

/// The unknown uarch errors are by far the most common error a user will encounter.
/// Rather than using the generic printBug function, which asks the user to report
/// the problem on the issues webpage, this function will check if the program is
/// the release version. In such case, support for this feature is most likely already
/// in the last version, so just tell the user to compile that one and not report this
/// in github.
void printBugCheckRelease(const char *fmt, ...) {
  int buffer_size = 4096;
  char buffer[buffer_size];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer,buffer_size, fmt, args);
  va_end(args);

  fprintf(stderr, RED "[ERROR]: "RESET "%s\n", buffer);
  fprintf(stderr, "[VERSION]: ");
  print_version(stderr);

#ifdef GIT_FULL_VERSION
  if (isReleaseVersion(GIT_FULL_VERSION)) {
    fprintf(stderr, RED "[ERROR]: "RESET "You are using an outdated version of cpufetch. Please compile cpufetch from source (see https://github.com/Dr-Noob/cpufetch?tab=readme-ov-file#22-building-from-source)");
  }
  else {
    printBugMessage(stderr);
  }
#else
  printBugMessage(stderr);
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

void print_version(FILE *restrict stream) {
#ifdef GIT_FULL_VERSION
  fprintf(stream, "cpufetch %s (%s %s)\n", GIT_FULL_VERSION, OS_STR, ARCH_STR);
#else
  fprintf(stream, "cpufetch v%s (%s %s)\n", VERSION, OS_STR, ARCH_STR);
#endif
}
