#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "../common/global.h"
#include "cpu.h"

#ifdef ARCH_X86
  #include "../x86/uarch.h"
#elif ARCH_PPC
  #include "../ppc/uarch.h"
#elif ARCH_ARM
  #include "../arm/uarch.h"
#endif

#define STRING_UNKNOWN    "Unknown"
#define STRING_YES        "Yes"
#define STRING_NO         "No"
#define STRING_NONE       "None"
#define STRING_MEGAHERZ   "MHz"
#define STRING_GIGAHERZ   "GHz"
#define STRING_KILOBYTES  "KB"
#define STRING_MEGABYTES  "MB"

VENDOR get_cpu_vendor(struct cpuInfo* cpu) {
  return cpu->cpu_vendor;
}

int64_t get_freq(struct frequency* freq) {
  return freq->max;
}

#if defined(ARCH_X86) || defined(ARCH_PPC)
char* get_str_cpu_name(struct cpuInfo* cpu) {
  return cpu->cpu_name;
}

char* get_str_sockets(struct topology* topo) {
  char* string = emalloc(sizeof(char) * 2);
  int32_t sanity_ret = snprintf(string, 2, "%d", topo->sockets);
  if(sanity_ret < 0) {
    printBug("get_str_sockets: snprintf returned a negative value for input: '%d'", topo->sockets);
    return NULL;
  }
  return string;
}

uint32_t get_nsockets(struct topology* topo) {
  return topo->sockets;
}
#endif

int32_t get_value_as_smallest_unit(char ** str, uint32_t value) {
  int32_t sanity_ret;
  *str = emalloc(sizeof(char)* 11); //8 for digits, 2 for units

  if(value/1024 >= 1024)
    sanity_ret = snprintf(*str, 10,"%.4g"STRING_MEGABYTES, (double)value/(1<<20));
  else
    sanity_ret = snprintf(*str, 10,"%.4g"STRING_KILOBYTES, (double)value/(1<<10));

  return sanity_ret;
}

// String functions
char* get_str_cache_two(int32_t cache_size, uint32_t physical_cores) {
  // 4 for digits, 2 for units, 2 for ' (', 3 digits, 2 for units and 7 for ' Total)'
  uint32_t max_size = 4+2 + 2 + 4+2 + 7 + 1;
  int32_t sanity_ret;
  char* string = emalloc(sizeof(char) * max_size);
  char* tmp1;
  char* tmp2;
  int32_t tmp1_len = get_value_as_smallest_unit(&tmp1, cache_size);
  int32_t tmp2_len = get_value_as_smallest_unit(&tmp2, cache_size * physical_cores);

  if(tmp1_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf returned a negative value for input: %d\n", cache_size);
    return NULL;
  }
  if(tmp2_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf returned a negative value for input: %d\n", cache_size * physical_cores);
    return NULL;
  }

  uint32_t size = tmp1_len + 2 + tmp2_len + 7 + 1;
  sanity_ret = snprintf(string, size, "%s (%s Total)", tmp1, tmp2);

  if(sanity_ret < 0) {
    printBug("get_str_cache_two: snprintf returned a negative value for input: '%s' and '%s'\n", tmp1, tmp2);
    return NULL;
  }

  free(tmp1);
  free(tmp2);
  return string;
}

char* get_str_cache_one(int32_t cache_size) {
  // 4 for digits, 2 for units, 2 for ' (', 3 digits, 2 for units and 7 for ' Total)'
  uint32_t max_size = 4+2 + 1;
  int32_t sanity_ret;
  char* string = emalloc(sizeof(char) * max_size);
  char* tmp;
  int32_t tmp_len = get_value_as_smallest_unit(&tmp, cache_size);

  if(tmp_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf returned a negative value for input: %d", cache_size);
    return NULL;
  }

  uint32_t size = tmp_len + 1;
  sanity_ret = snprintf(string, size, "%s", tmp);

  if(sanity_ret < 0) {
    printBug("get_str_cache_one: snprintf returned a negative value for input: '%s'", tmp);
    return NULL;
  }
  free(tmp);
  return string;
}

char* get_str_cache(int32_t cache_size, int32_t num_caches) {
  if(num_caches > 1)
    return get_str_cache_two(cache_size, num_caches);
  else
    return get_str_cache_one(cache_size);
}

char* get_str_l1i(struct cache* cach) {
  return get_str_cache(cach->L1i->size, cach->L1i->num_caches);
}

char* get_str_l1d(struct cache* cach) {
  return get_str_cache(cach->L1d->size, cach->L1d->num_caches);
}

char* get_str_l2(struct cache* cach) {
  assert(cach->L2->exists);
  return get_str_cache(cach->L2->size, cach->L2->num_caches);
}

char* get_str_l3(struct cache* cach) {
  if(!cach->L3->exists)
    return NULL;
  return get_str_cache(cach->L3->size, cach->L3->num_caches);
}

char* get_str_freq(struct frequency* freq) {
  //Max 3 digits and 3 for '(M/G)Hz' plus 1 for '\0'
  uint32_t size = (5+1+3+1);
  assert(strlen(STRING_UNKNOWN)+1 <= size);
  char* string = emalloc(sizeof(char)*size);
  memset(string, 0, sizeof(char)*size);

  if(freq->max == UNKNOWN_FREQ || freq->max < 0)
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
  else if(freq->max >= 1000)
    snprintf(string,size,"%.3f "STRING_GIGAHERZ,(float)(freq->max)/1000);
  else
    snprintf(string,size,"%d "STRING_MEGAHERZ,freq->max);

  return string;
}

void free_cache_struct(struct cache* cach) {
  for(int i=0; i < 4; i++) free(cach->cach_arr[i]);
  free(cach->cach_arr);
  free(cach);
}

void free_freq_struct(struct frequency* freq) {
  free(freq);
}

void free_hv_struct(struct hypervisor* hv) {
  free(hv);
}

void free_cpuinfo_struct(struct cpuInfo* cpu) {
  free_uarch_struct(cpu->arch);
  free_hv_struct(cpu->hv);
  #ifdef ARCH_X86
  free(cpu->cpu_name);
  #endif
  free(cpu);
}
