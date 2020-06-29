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

#define TITLE_NAME        "Name:"
#define TITLE_FREQUENCY   "Frequency:"
#define TITLE_SOCKETS     "Sockets:" 
#define TITLE_NCORES      "Cores:" 
#define TITLE_NCORES_DUAL "Cores (Total):"
#define TITLE_AVX         "AVX:"
#define TITLE_SSE         "SSE:"
#define TITLE_FMA         "FMA:"
#define TITLE_AES         "AES:"
#define TITLE_SHA         "SHA:"
#define TITLE_L1i         "L1i Size:"
#define TITLE_L1d         "L1d Size:"
#define TITLE_L2          "L2 Size:"
#define TITLE_L3          "L3 Size:"
#define TITLE_PEAK        "Peak Perf.:"

#define MAX_ATTRIBUTE_COUNT      15
#define ATTRIBUTE_NAME            0
#define ATTRIBUTE_FREQUENCY       1
#define ATTRIBUTE_SOCKETS         2
#define ATTRIBUTE_NCORES          3
#define ATTRIBUTE_NCORES_DUAL     4
#define ATTRIBUTE_AVX             5
#define ATTRIBUTE_SSE             6
#define ATTRIBUTE_FMA             7
#define ATTRIBUTE_AES             8
#define ATTRIBUTE_SHA             9
#define ATTRIBUTE_L1i            10
#define ATTRIBUTE_L1d            11
#define ATTRIBUTE_L2             12
#define ATTRIBUTE_L3             13
#define ATTRIBUTE_PEAK           14

static const int STYLES_CODE_LIST [STYLES_COUNT] = {STYLE_DEFAULT, STYLE_DARK};

static const char* ATTRIBUTE_FIELDS [MAX_ATTRIBUTE_COUNT] = { TITLE_NAME, TITLE_FREQUENCY, TITLE_SOCKETS,
                                                              TITLE_NCORES, TITLE_NCORES_DUAL, 
                                                              TITLE_AVX, TITLE_SSE,
                                                              TITLE_FMA, TITLE_AES, TITLE_SHA,
                                                              TITLE_L1i, TITLE_L1d, TITLE_L2, TITLE_L3,
                                                              TITLE_PEAK
                                                               };

static const int ATTRIBUTE_LIST[MAX_ATTRIBUTE_COUNT] =  { ATTRIBUTE_NAME, ATTRIBUTE_FREQUENCY, ATTRIBUTE_SOCKETS,
                                                        ATTRIBUTE_NCORES, ATTRIBUTE_NCORES_DUAL, ATTRIBUTE_AVX,
                                                        ATTRIBUTE_SSE, ATTRIBUTE_FMA, ATTRIBUTE_AES, ATTRIBUTE_SHA,
                                                        ATTRIBUTE_L1i, ATTRIBUTE_L1d, ATTRIBUTE_L2, ATTRIBUTE_L3,
                                                        ATTRIBUTE_PEAK };

struct ascii {
  char art[NUMBER_OF_LINES][LINE_SIZE];
  char color1[10];
  char color2[10];
  char reset[10];
  char* attributes[MAX_ATTRIBUTE_COUNT];
  uint32_t n_attributes_set;
  VENDOR vendor;
};

void setAttribute(struct ascii* art, int type, char* value) {
  art->attributes[type] = value;  
  art->n_attributes_set++;
}

struct ascii* set_ascii(VENDOR cpuVendor, STYLE style) {
  // Sanity checks //
  for(int i=0; i < MAX_ATTRIBUTE_COUNT; i++) {
    if(ATTRIBUTE_FIELDS[i] == NULL) {
      printBug("Attribute field at position %d is empty", i);    
      return NULL;
    }
    if(i > 0 && ATTRIBUTE_LIST[i] == 0) {
      printBug("Attribute list at position %d is empty", i);    
      return NULL;
    }
  }
  
  char *COL_DEFAULT_1, *COL_DEFAULT_2, *COL_DARK_1, *COL_DARK_2;
  struct ascii* art = malloc(sizeof(struct ascii));
  art->n_attributes_set = 0;
  art->vendor = cpuVendor;
  for(int i=0; i < MAX_ATTRIBUTE_COUNT; i++)
    art->attributes[i] = NULL;
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

uint32_t get_next_attribute(struct ascii* art, uint32_t last_attr) {
  last_attr++;
  while(art->attributes[last_attr] == NULL) last_attr++;
  return last_attr;
}

void print_ascii_intel(struct ascii* art, uint32_t la) {
  bool flag = false;
  int attr_to_print = -1;
  uint32_t space_right;
  uint32_t space_up = (NUMBER_OF_LINES - art->n_attributes_set)/2;
  uint32_t space_down = NUMBER_OF_LINES - art->n_attributes_set - space_up;

  for(uint32_t n=0;n<NUMBER_OF_LINES;n++) {

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

    
    if(n > space_up-1 && n < NUMBER_OF_LINES-space_down) {      
      attr_to_print = get_next_attribute(art, attr_to_print);
      space_right = 1 + (la - strlen(ATTRIBUTE_FIELDS[attr_to_print]));      
      printf("%s%s%*s%s%s%s\n",art->color1, ATTRIBUTE_FIELDS[attr_to_print], space_right, "", art->color2, art->attributes[attr_to_print], art->reset);
    }
    else printf("\n");
  }
}

void print_ascii_amd(struct ascii* art, uint32_t la) {
  int attr_to_print = -1;
  uint32_t space_up = (NUMBER_OF_LINES - art->n_attributes_set)/2;
  uint32_t space_down = NUMBER_OF_LINES - art->n_attributes_set - space_up;
  
  for(uint32_t n=0;n<NUMBER_OF_LINES;n++) {
    for(int i=0;i<LINE_SIZE;i++) {
      if(art->art[n][i] == '@')
        printf("%s%c%s", art->color1, art->art[n][i], art->reset);
      else if(art->art[n][i] == '#')
        printf("%s%c%s", art->color2, art->art[n][i], art->reset);
      else
        printf("%c",art->art[n][i]);
    }

    if(n > space_up-1 && n < NUMBER_OF_LINES-space_down) {
      attr_to_print = get_next_attribute(art, attr_to_print);
      printf("%s%s%s%s%s\n",art->color1, ATTRIBUTE_FIELDS[attr_to_print], art->color2, art->attributes[attr_to_print], art->reset);
    }
    else printf("\n");
  }

}

uint32_t longest_attribute_length(struct ascii* art) {
  uint32_t max = 0;
  uint64_t len = 0;
  
  for(int i=0; i < MAX_ATTRIBUTE_COUNT; i++) {
    if(art->attributes[i] != NULL) {
      len = strlen(ATTRIBUTE_FIELDS[i]);
      if(len > max) max = len;
    }
  }
  
  return max;
}

void print_ascii(struct ascii* art) {
  uint32_t longest_attribute = longest_attribute_length(art);
  if(art->vendor == VENDOR_INTEL)
    print_ascii_intel(art, longest_attribute);
  else
    print_ascii_amd(art, longest_attribute);
}

bool print_cpufetch(struct cpuInfo* cpu, struct cache* cach, struct frequency* freq, struct topology* topo, STYLE s) {
  struct ascii* art = set_ascii(get_cpu_vendor(cpu), s);
  if(art == NULL)
    return false;
  
  char* cpu_name = get_str_cpu_name(cpu);
  char* sockets = get_str_sockets(topo);
  char* max_frequency = get_str_freq(freq);
  char* n_cores = get_str_topology(topo, false);
  char* n_cores_dual = get_str_topology(topo, true);
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
  setAttribute(art,ATTRIBUTE_NCORES,n_cores);
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
  
  uint32_t socket_num = get_nsockets(topo);
  if (socket_num > 1) {    
    setAttribute(art, ATTRIBUTE_SOCKETS, sockets);
    setAttribute(art, ATTRIBUTE_NCORES_DUAL, n_cores_dual);
  }
  
  if(art->n_attributes_set > NUMBER_OF_LINES) {
    printBug("The number of attributes set is bigger than the max that can be displayed");
    return false;    
  }

  print_ascii(art);

  free(cpu_name);
  free(max_frequency);
  free(sockets);
  free(n_cores);
  free(n_cores_dual);
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
