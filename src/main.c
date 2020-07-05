#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "printer.h"
#include "cpuid.h"
#include "global.h"

static const char* VERSION = "0.56";

void print_help(char *argv[]) {
  printf("Usage: %s [--version] [--help] [--levels] [--style fancy|retro] [--color 'R,G,B:R,G,B:R,G,B:R,G,B']\n\
Options: \n\
  --color    Set text color. 4 colors (in RGB format) must be specified in the form: R,G,B:R,G,B:...\n\
             These colors correspond to the ASCII art color (2 colors) and for the text colors (next 2)\n\
             Suggested color (Intel): --color 15,125,194:230,230,230:40,150,220:230,230,230\n\
  --style    Set the style of the ASCII art:\n\
    * fancy \n\
    * retro \n\
  --help     Prints this help and exit\n\
  --levels   Prints CPU model and cpuid levels (debug purposes)\n\
  --version  Prints cpufetch version and exit\n",
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
    print_levels(cpu, get_str_cpu_name(cpu));
    return EXIT_SUCCESS;    
  }

  struct cache* cach = get_cache_info(cpu);
  if(cach == NULL)
    return EXIT_FAILURE;
  
  struct frequency* freq = get_frequency_info(cpu);
  if(freq == NULL)
    return EXIT_FAILURE;
  
  struct topology* topo = get_topology_info(cpu);
  if(topo == NULL)
    return EXIT_FAILURE;
  
  if(print_cpufetch(cpu, cach, freq, topo, get_style(), get_colors()))  
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
