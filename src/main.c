#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "printer.h"
#include "cpuid.h"
#include "global.h"

/***
                              ################                
                      #######                #######          
                 ####                              ####       
             ###                                     ####     
        ###                                             ###   Name:       Intel(R) Xeon(R) CPU E5-2698 v4 @ 2.20GHz
        ###                                             ###   Sockets:    2
                                                              Cores:      20 (40 threads)
                                                              Frequency:  3.60 GHz
   ##   ###   #########   ######   ######    ###        ###   AVX:        AVX,AVX2
  ##    ###   ###    ###  ###    ####  ####  ###        ###   SSE:        SSE,SSE2,SSE3,SSSE3,SSE4_1,SSE4_2
 ##     ###   ###    ###  ###    ###    ###  ###       ###    FMA:        FMA3
##       ##   ###    ###   #####  #########   ##  ###         L1d Size:   32KB  (1MB Total)
##                                                ##          L1i Size:   32KB  (1MB Total)
###                                                           L2  Size:   256KB (4MB Total)
 ###                                                          L3  Size:   50M   (100MB Total)
 ####                                        ####             Peak Perf.: 2.30 TFLOP/s
   #####                               ##########             
     ##########               ################                
         ###############################    

***/

static const char* VERSION = "0.411";

void print_help(char *argv[]) {
  printf("Usage: %s [--version] [--help] [--style STYLE]\n\
Options: \n\
  --style    Set logo style color\n\
    default:   Default style color\n\
    dark:      Dark style color\n\
    none:      Don't use colors\n\
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
  
  if(print_cpufetch(cpu, cach, freq, topo, get_style()))  
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
