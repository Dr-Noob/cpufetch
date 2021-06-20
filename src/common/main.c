#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "printer.h"
#include "global.h"

#ifdef ARCH_X86
  static const char* ARCH_STR = "x86_64 build";
  #include "../x86/cpuid.h"
#elif ARCH_ARM
  static const char* ARCH_STR = "ARM build";
  #include "../arm/midr.h"
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

static const char* VERSION = "0.98";

void print_help(char *argv[]) {
  const char **t = args_str;
  const char *c = args_chr;  
  int max_len = max_arg_str_length();

  printf("Usage: %s [OPTION]...\n", argv[0]);
  printf("Simple yet fancy CPU architecture fetching tool\n\n");

  printf("Options: \n");
  printf("  -%c, --%s %*s Set the color scheme (by default, cpufetch uses the system color scheme)\n", c[ARG_COLOR], t[ARG_COLOR], (int) (max_len-strlen(t[ARG_COLOR])), "");
  printf("  -%c, --%s %*s Set the style of CPU art\n", c[ARG_STYLE], t[ARG_STYLE], (int) (max_len-strlen(t[ARG_STYLE])), "");
#ifdef ARCH_X86
  printf("  -%c, --%s %*s Prints CPU model and cpuid levels (debug purposes)\n", c[ARG_DEBUG], t[ARG_DEBUG], (int) (max_len-strlen(t[ARG_DEBUG])), "");
#elif ARCH_ARM
  printf("  -%c, --%s %*s Prints main ID register values for all cores (debug purposes)\n", c[ARG_DEBUG], t[ARG_DEBUG], (int) (max_len-strlen(t[ARG_DEBUG])), "");
#endif
  printf("  -%c, --%s %*s Prints extra information (if available) about how cpufetch tried fetching information\n", c[ARG_VERBOSE], t[ARG_VERBOSE], (int) (max_len-strlen(t[ARG_VERBOSE])), "");
#ifdef ARCH_X86
  printf("  -%c, --%s %*s Prints raw cpuid data\n", c[ARG_RAW], t[ARG_RAW], (int) (max_len-strlen(t[ARG_RAW])), "");
#endif
  printf("  -%c, --%s %*s Prints this help and exit\n", c[ARG_HELP], t[ARG_HELP], (int) (max_len-strlen(t[ARG_HELP])), "");
  printf("  -%c, --%s %*s Prints cpufetch version and exit\n", c[ARG_VERSION], t[ARG_VERSION], (int) (max_len-strlen(t[ARG_VERSION])), "");
  
  printf("\nCOLORS: \n");
  printf("  * \"intel\":     Use Intel default color scheme \n");
  printf("  * \"amd\":       Use AMD default color scheme \n");
  printf("  * \"arm\":       Use ARM default color scheme \n");
  printf("  * custom:      If color argument do not match \"intel\", \"amd\" or \"arm\", a custom scheme can be specified.\n");
  printf("                 4 colors must be given in RGB with the format: R,G,B:R,G,B:...\n");
  printf("                 The first 2 colors are the CPU art color and the next 2 colors are the text colors\n");
  
  printf("\nSTYLES: \n");
  printf("  * \"fancy\":     Default style\n");
  printf("  * \"retro\":     Old cpufetch style\n");
  printf("  * \"legacy\":    Fallback style for terminals that do not support colors\n");
  
  printf("\nEXAMPLES: \n");
  printf("  Run cpufetch with Intel color scheme:\n");
  printf("    ./cpufetch --color intel\n");
  printf("  Run cpufetch with a custom color scheme:\n");
  printf("    ./cpufetch --color 239,90,45:210,200,200:100,200,45:0,200,200\n");
  
  printf("\nBUGS: \n");
  printf("    Report bugs to https://github.com/Dr-Noob/cpufetch/issues\n");
  
  printf("\nNOTE: \n");
  printf("    Peak performance information is NOT accurate. cpufetch computes peak performance using the max\n");
  printf("    frequency. However, to properly compute peak performance, you need to know the frequency of the\n");
  printf("    CPU running AVX code, which is not be fetched by cpufetch since it depends on each specific CPU.\n");
  printf("    For peak performance measurement see: https://github.com/Dr-Noob/peakperf\n");
}

void print_version() {
  printf("cpufetch v%s (%s %s)\n",VERSION, OS_STR, ARCH_STR);
}

int main(int argc, char* argv[]) {
  if(!parse_args(argc,argv))
    return EXIT_FAILURE;

  if(show_help()) {
    print_help(argv);
    return EXIT_SUCCESS;
  }

  if(show_version()) {
    print_version();
    return EXIT_SUCCESS;
  }

  set_log_level(verbose_enabled());

  struct cpuInfo* cpu = get_cpu_info();
  if(cpu == NULL)
    return EXIT_FAILURE;

  if(show_debug()) {
    print_version();
    print_debug(cpu);
    return EXIT_SUCCESS;
  }
  
  if(show_raw()) {
  #ifdef ARCH_X86
    print_version();
    print_raw(cpu);
    return EXIT_SUCCESS;
  #else
    printErr("raw option is valid only in x86_64");
    return EXIT_FAILURE;
  #endif
  }
  
  if(print_cpufetch(cpu, get_style(), get_colors()))
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
