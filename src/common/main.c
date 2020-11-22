#include <stdio.h>
#include <stdlib.h>

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

static const char* VERSION = "0.87";

void print_help(char *argv[]) {
  printf("Usage: %s [--version] [--help] [--debug] [--style \"fancy\"|\"retro\"|\"legacy\"] [--color \"intel\"|\"amd\"|'R,G,B:R,G,B:R,G,B:R,G,B']\n\n", argv[0]);

  printf("Options: \n\
  --color       Set the color scheme. By default, cpufetch uses the system color scheme. This option \n\
                lets the user use different colors to print the CPU art:  \n\
   * \"intel\":   Use Intel default color scheme \n\
   * \"amd\":     Use AMD default color scheme \n\
   * \"arm\":     Use ARM default color scheme \n\
   * custom:    If color argument do not match \"Intel\", \"AMD\" or \"ARM\", a custom scheme can be specified: \n\
                4 colors must be given in RGB with the format: R,G,B:R,G,B:... \n\
                These colors correspond to CPU art color (2 colors) and for the text colors (following 2) \n\
                For example: --color 239,90,45:210,200,200:100,200,45:0,200,200 \n\n\
  --style       Set the style of CPU art: \n\
    * \"fancy\":  Default style      \n\
    * \"retro\":  Old cpufetch style \n\
    * \"legacy\": Fallback style for terminals that does not support colors                       \n\n");

#ifdef ARCH_X86
  printf("  --debug       Prints CPU model and cpuid levels (debug purposes)\n\n");
#elif ARCH_ARM
  printf("  --debug       Prints main ID register values for all cores (debug purposes)\n\n");
#endif

  printf("  --verbose     Prints extra information (if available) about how cpufetch tried fetching information\n\n\
  --help        Prints this help and exit\n\n\
  --version     Prints cpufetch version and exit\n\n\
                                                   \n\
NOTES: \n\
  - Bugs or improvements should be submitted to: github.com/Dr-Noob/cpufetch/issues        \n\
  - Peak performance information is NOT accurate. cpufetch computes peak performance using the max   \n\
    frequency. However, to properly compute peak performance, you need to know the frequency of the  \n\
    CPU running AVX code, which is not be fetched by cpufetch since it depends on each specific CPU. \n");
}

void print_version() {
  printf("cpufetch v%s (%s)\n",VERSION, ARCH_STR);
}

int main(int argc, char* argv[]) {
  if(!parse_args(argc,argv))
    return EXIT_FAILURE;

  if(show_help()) {
    print_version();
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
  
  if(print_cpufetch(cpu, get_style(), get_colors()))
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
