#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "printer.h"
#include "ascii.h"
#include "global.h"

#define COL_NONE            ""
#define COL_INTEL_DEFAULT_1 "\x1b[36;1m"
#define COL_INTEL_DEFAULT_2 "\x1b[37;1m"
#define COL_INTEL_DARK_1    "\x1b[34;1m"
#define COL_INTEL_DARK_2    "\x1b[30m"
#define COL_AMD_DEFAULT_1   "\x1b[37;1m"
#define COL_AMD_DEFAULT_2   "\x1b[31;1m"
#define COL_AMD_DARK_1      "\x1b[30;1m"
#define COL_AMD_DARK_2      "\x1b[32;1m"
#define RESET               "\x1b[0m"

#define TITLE_NAME      "Name:       "
#define TITLE_FREQUENCY "Frequency:  "
#define TITLE_NCORES    "N.Cores:    "
#define TITLE_AVX       "AVX:        "
#define TITLE_SSE       "SSE:        "
#define TITLE_FMA       "FMA:        "
#define TITLE_AES       "AES:        "
#define TITLE_SHA       "SHA:        "
#define TITLE_L1i       "L1i Size:   "
#define TITLE_L1d       "L1d Size:   "
#define TITLE_L2        "L2 Size:    "
#define TITLE_L3        "L3 Size:    "
#define TITLE_PEAK      "Peak FLOPS: "

/*** CENTER TEXT ***/
#define LINES_SPACE_UP   3
#define LINES_SPACE_DOWN 4

#define ATTRIBUTE_COUNT    13
#define ATTRIBUTE_NAME      0
#define ATTRIBUTE_FREQUENCY 1
#define ATTRIBUTE_NCORES    2
#define ATTRIBUTE_AVX       3
#define ATTRIBUTE_SSE       4
#define ATTRIBUTE_FMA       5
#define ATTRIBUTE_AES       6
#define ATTRIBUTE_SHA       7
#define ATTRIBUTE_L1i       8
#define ATTRIBUTE_L1d       9
#define ATTRIBUTE_L2        10
#define ATTRIBUTE_L3        11
#define ATTRIBUTE_PEAK      12

static const int STYLES_CODE_LIST [STYLES_COUNT] = {STYLE_DEFAULT, STYLE_DARK};

static const char* ATTRIBUTE_FIELDS [ATTRIBUTE_COUNT] =  {  TITLE_NAME, TITLE_FREQUENCY,
                                                            TITLE_NCORES, TITLE_AVX, TITLE_SSE,
                                                            TITLE_FMA, TITLE_AES, TITLE_SHA,
                                                            TITLE_L1i, TITLE_L1d, TITLE_L2, TITLE_L3,
                                                            TITLE_PEAK };

static const int ATTRIBUTE_LIST[ATTRIBUTE_COUNT] =  { ATTRIBUTE_NAME, ATTRIBUTE_FREQUENCY,
                                                        ATTRIBUTE_NCORES, ATTRIBUTE_AVX, ATTRIBUTE_SSE,
                                                        ATTRIBUTE_FMA, ATTRIBUTE_AES, ATTRIBUTE_SHA,
                                                        ATTRIBUTE_L1i, ATTRIBUTE_L1d, ATTRIBUTE_L2, ATTRIBUTE_L3,
                                                        ATTRIBUTE_PEAK };

struct ascii {
  char art[NUMBER_OF_LINES][LINE_SIZE];
  char color1[10];
  char color2[10];
  char reset[10];
  char* atributes[ATTRIBUTE_COUNT];
  VENDOR vendor;
};

void setAttribute(struct ascii* art, int type, char* value) {
  int i = 0;
  while(i < ATTRIBUTE_COUNT && type != ATTRIBUTE_LIST[i])
    i++;
  if(i != ATTRIBUTE_COUNT)
    art->atributes[i] = value;
  else
    printBug("Setting attribute failed because it was not found");
}

struct ascii* set_ascii(VENDOR cpuVendor, STYLE style) {
  /*** Check that number of lines of ascii art matches the number
  of spaces plus the number of lines filled with text ***/
  if(LINES_SPACE_UP+LINES_SPACE_DOWN+ATTRIBUTE_COUNT != NUMBER_OF_LINES) {
    printBug("Number of lines do not match (%d vs %d)",LINES_SPACE_UP+LINES_SPACE_DOWN+ATTRIBUTE_COUNT,NUMBER_OF_LINES);
    return NULL;
  }

  char *COL_DEFAULT_1, *COL_DEFAULT_2, *COL_DARK_1, *COL_DARK_2;
  struct ascii* art = malloc(sizeof(struct ascii));
  art->vendor = cpuVendor;
  strcpy(art->reset,RESET);
  
  if(cpuVendor == VENDOR_INTEL) {
    COL_DEFAULT_1 = COL_INTEL_DEFAULT_1;
    COL_DEFAULT_2 = COL_INTEL_DEFAULT_2;
    COL_DARK_1 = COL_INTEL_DARK_1;
    COL_DARK_2 = COL_INTEL_DARK_2;
  }
  else {
    COL_DEFAULT_1 = COL_AMD_DEFAULT_1;
    COL_DEFAULT_2 = COL_AMD_DEFAULT_2;
    COL_DARK_1 = COL_AMD_DARK_1;
    COL_DARK_2 = COL_AMD_DARK_2;    
  }
  
  switch(style) {
    case STYLE_NONE:
        strcpy(art->color1,COL_NONE);
        strcpy(art->color2,COL_NONE);
        break; 
    case STYLE_EMPTY:
      #ifdef _WIN32
        strcpy(art->color1,COL_NONE);
        strcpy(art->color2,COL_NONE);  
        art->reset[0] = '\0';
        break;
      #endif
    case STYLE_DEFAULT:
      strcpy(art->color1,COL_DEFAULT_1);
      strcpy(art->color2,COL_DEFAULT_2);
      break;
    case STYLE_DARK:
      strcpy(art->color1,COL_DARK_1);
      strcpy(art->color2,COL_DARK_2);
      break;
    default:
      printBug("Found invalid style (%d)",style);
      return NULL;    
  }
  
  char tmp[NUMBER_OF_LINES*LINE_SIZE];
  if(cpuVendor == VENDOR_INTEL) strcpy(tmp, INTEL_ASCII);    
  else strcpy(tmp, AMD_ASCII);    
    for(int i=0; i < NUMBER_OF_LINES; i++)
      strncpy(art->art[i], tmp + i*LINE_SIZE, LINE_SIZE); 
    
  return art;
}

void print_ascii_intel(struct ascii* art) {
  bool flag = false;

  for(int n=0;n<NUMBER_OF_LINES;n++) {

    /*** PRINT ASCII-ART ***/
    for(int i=0;i<LINE_SIZE;i++) {
      if(flag) {
        if(art->art[n][i] == ' ') {
          flag = false;
          printf("%c",art->art[n][i]);
        }
        else
          printf("%s%c%s", art->color1, art->art[n][i], art->reset);
      }
      else {
        if(art->art[n][i] != ' ') {
          flag = true;
          printf("%s%c%s", art->color2, art->art[n][i], art->reset);
        }
        else
          printf("%c",art->art[n][i]);
      }
    }

    /*** PRINT ATTRIBUTE ***/
    if(n>LINES_SPACE_UP-1 && n<NUMBER_OF_LINES-LINES_SPACE_DOWN)
      printf("%s%s%s%s%s\n",art->color1,ATTRIBUTE_FIELDS[n-LINES_SPACE_UP],art->color2,art->atributes[n-LINES_SPACE_UP],art->reset);
    else printf("\n");
  }
}

void print_ascii_amd(struct ascii* art) {

  for(int n=0;n<NUMBER_OF_LINES;n++) {
    /*** PRINT ASCII-ART ***/
    for(int i=0;i<LINE_SIZE;i++) {
      if(art->art[n][i] == '@')
        printf("%s%c%s", art->color1, art->art[n][i], art->reset);
      else if(art->art[n][i] == '#')
        printf("%s%c%s", art->color2, art->art[n][i], art->reset);
      else
        printf("%c",art->art[n][i]);
    }

    /*** PRINT ATTRIBUTE ***/
    if(n>LINES_SPACE_UP-1 && n<NUMBER_OF_LINES-LINES_SPACE_DOWN)
      printf("%s%s%s%s%s\n",art->color1,ATTRIBUTE_FIELDS[n-LINES_SPACE_UP],art->color2,art->atributes[n-LINES_SPACE_UP], art->reset);
    else printf("\n");
  }

}

void print_ascii(struct ascii* art) {
  if(art->vendor == VENDOR_INTEL)
    print_ascii_intel(art);
  else
    print_ascii_amd(art);
}

bool print_cpufetch(struct cpuInfo* cpu, struct cache* cach, struct frequency* freq, struct topology* topo, STYLE s) {
  struct ascii* art = set_ascii(get_cpu_vendor(cpu),s);
  if(art == NULL)
    return false;
  
  char* cpu_name = get_str_cpu_name(cpu);
  char* max_frequency = get_str_freq(freq);
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

  setAttribute(art,ATTRIBUTE_NAME,cpu_name);
  setAttribute(art,ATTRIBUTE_FREQUENCY,max_frequency);
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

  free(cpu_name);
  free(max_frequency);
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
  
  return true;
}
