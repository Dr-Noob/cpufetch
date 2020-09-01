#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "printer.h"
#include "cpuid.h"
#include "global.h"

static const char* VERSION = "0.66";

void print_help(char *argv[]) {
  printf("Usage: %s [--version] [--help] [--levels] [--style \"fancy\"|\"retro\"|\"legacy\"] [--color \"intel\"|\"amd\"|'R,G,B:R,G,B:R,G,B:R,G,B']\n\n\
Options: \n\
  --color       Set the color scheme. By default, cpufetch uses the system color scheme. This option \n\
                lets the user use different colors to print the CPU art:  \n\
   * \"intel\":   Use to intel color scheme \n\
   * \"amd\":     Use amd default color scheme \n\
   * custom:    If color do not match \"intel\" or \"amd\", a custom scheme can be specified: \n\
                4 colors must be given in RGB with the format: R,G,B:R,G,B:... \n\
                These colors correspond to CPU art color (2 colors) and for the text colors (following 2) \n\
                For example: --color 239,90,45:210,200,200:100,200,45:0,200,200 \n\n\
  --style       Set the style of CPU art: \n\
    * \"fancy\"   (default style)      \n\
    * \"retro\"   (old cpufetch style) \n\
    * \"legacy\"                       \n\n\
  --levels      Prints CPU model and cpuid levels (debug purposes)\n\n\
  --verbose     Prints extra information (if available) about how cpufetch tried fetching information\n\n\
  --help        Prints this help and exit\n\n\
  --version     Prints cpufetch version and exit\n\n\
                                                   \n\
NOTES: \n\
  - Bugs or improvements should be submitted to: github.com/Dr-Noob/cpufetch/issues        \n\
  - Peak performance information is NOT accurate. cpufetch computes peak performance using the max   \n\
    frequency. However, to properly compute peak performance, you need to know the frequency of the  \n\
    CPU running AVX code, which is not be fetched by cpufetch since it depends on each specific CPU. \n",
  argv[0]);
}

void print_version() {
  printf("cpufetch v%s\n",VERSION);
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
  
  if(show_levels()) {
    print_version();
    print_levels(cpu);
    return EXIT_SUCCESS;    
  }  
  
  struct frequency* freq = get_frequency_info(cpu);
  if(freq == NULL)
    return EXIT_FAILURE;
  
  struct cache* cach = get_cache_info(cpu);
  if(cach == NULL)
    return EXIT_FAILURE;
  
  struct topology* topo = get_topology_info(cpu, cach);
  if(topo == NULL)
    return EXIT_FAILURE;
  
  if(print_cpufetch(cpu, cach, freq, topo, get_style(), get_colors()))  
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
