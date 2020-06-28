#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "printer.h"
#include "standart.h"
#include "extended.h"
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
  char* cpuName = get_str_cpu_name();
  
  if(show_levels()) {
    print_version();
    print_levels(cpu, cpuName);
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
  
  struct ascii* art = set_ascii(get_cpu_vendor(cpu),get_style());
  if(art == NULL)
    return EXIT_FAILURE;
  
  char* maxFrequency = get_str_freq(freq);
  char* nCores = get_str_topology(topo);
  char* avx = get_str_avx(cpu);
  char* sse = get_str_sse(cpu);
  char* fma = get_str_fma(cpu);
  char* aes = get_str_aes(cpu);
  char* sha = get_str_sha(cpu);
  char* l1i = get_str_l1i(cach, topo);
  char* l1d = get_str_l1d(cach, topo);
  char* l2 = get_str_l2(cach, topo);
  char* l3 = get_str_l3(cach, topo);
  char* pp = get_str_peak_performance(cpu,topo,get_freq(freq));

  setAttribute(art,ATTRIBUTE_NAME,cpuName);
  setAttribute(art,ATTRIBUTE_FREQUENCY,maxFrequency);
  setAttribute(art,ATTRIBUTE_NCORES,nCores);
  setAttribute(art,ATTRIBUTE_AVX,avx);
  setAttribute(art,ATTRIBUTE_SSE,sse);
  setAttribute(art,ATTRIBUTE_FMA,fma);
  setAttribute(art,ATTRIBUTE_AES,aes);
  setAttribute(art,ATTRIBUTE_SHA,sha);
  setAttribute(art,ATTRIBUTE_L1i,l1i);
  setAttribute(art,ATTRIBUTE_L1d,l1d);
  setAttribute(art,ATTRIBUTE_L2,l2);
  setAttribute(art,ATTRIBUTE_L3,l3);
  setAttribute(art,ATTRIBUTE_PEAK,pp);

  print_ascii(art);

  free(cpuName);
  free(maxFrequency);
  free(nCores);
  free(avx);
  free(sse);
  free(fma);
  free(aes);
  free(sha);
  free(l1i);
  free(l1d);
  free(l2);
  free(l3);
  free(pp);

  free(cpu);
  free(art);
  free_cache_struct(cach);
  free_topo_struct(topo);
  free_freq_struct(freq);

  return EXIT_SUCCESS;
}
