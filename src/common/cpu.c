#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "../common/global.h"
#include "cpu.h"

#ifdef ARCH_X86
  #include "../x86/uarch.h"
  #include "../x86/apic.h"
#elif ARCH_PPC
  #include "../ppc/uarch.h"
#elif ARCH_ARM
  #include "../arm/uarch.h"
#endif

#define UNUSED(x) (void)(x)

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
char* get_str_cpu_name(struct cpuInfo* cpu, bool fcpuname) {
  #ifdef ARCH_X86
  if(!fcpuname) {
    return get_str_cpu_name_abbreviated(cpu);
  }
  #elif ARCH_PPC
  UNUSED(fcpuname);
  #endif
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
  int32_t ret;
  int max_len = 10; // Max is 8 for digits, 2 for units
  *str = emalloc(sizeof(char)* (max_len + 1));

  if(value/1024 >= 1024)
    ret = snprintf(*str, max_len, "%.4g"STRING_MEGABYTES, (double)value/(1<<20));
  else
    ret = snprintf(*str, max_len, "%.4g"STRING_KILOBYTES, (double)value/(1<<10));

  return ret;
}

// String functions
char* get_str_cache_two(int32_t cache_size, uint32_t physical_cores) {
  char* tmp1;
  char* tmp2;
  int32_t tmp1_len = get_value_as_smallest_unit(&tmp1, cache_size);
  int32_t tmp2_len = get_value_as_smallest_unit(&tmp2, cache_size * physical_cores);

  // tmp1_len for first output, 2 for ' (', tmp2_len for second output and 7 for ' Total)'
  uint32_t size = tmp1_len + 2 + tmp2_len + 7 + 1;
  char* string = emalloc(sizeof(char) * size);

  if(tmp1_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf failed for input: %d\n", cache_size);
    return NULL;
  }
  if(tmp2_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf failed for input: %d\n", cache_size * physical_cores);
    return NULL;
  }

  if(snprintf(string, size, "%s (%s Total)", tmp1, tmp2) < 0) {
    printBug("get_str_cache_two: snprintf failed for input: '%s' and '%s'\n", tmp1, tmp2);
    return NULL;
  }

  free(tmp1);
  free(tmp2);

  return string;
}

char* get_str_cache_one(int32_t cache_size) {
  char* string;
  int32_t str_len = get_value_as_smallest_unit(&string, cache_size);

  if(str_len < 0) {
    printBug("get_value_as_smallest_unit: snprintf failed for input: %d", cache_size);
    return NULL;
  }

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

  if(freq->max == UNKNOWN_DATA || freq->max < 0)
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
  else if(freq->max >= 1000)
    snprintf(string,size,"%.3f "STRING_GIGAHERZ,(float)(freq->max)/1000);
  else
    snprintf(string,size,"%d "STRING_MEGAHERZ,freq->max);

  return string;
}

char* get_str_peak_performance(int64_t flops) {
  char* str;

  if(flops == -1) {
    str = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN) + 1));
    strncpy(str, STRING_UNKNOWN, strlen(STRING_UNKNOWN) + 1);
    return str;
  }

  // 7 for digits (e.g, XXXX.XX), 7 for XFLOP/s
  double flopsd = (double) flops;
  uint32_t max_size = 7+1+7+1;
  str = ecalloc(max_size, sizeof(char));

  if(flopsd >= (double)1000000000000.0)
    snprintf(str, max_size, "%.2f TFLOP/s", flopsd/1000000000000);
  else if(flopsd >= 1000000000.0)
    snprintf(str, max_size, "%.2f GFLOP/s", flopsd/1000000000);
  else
    snprintf(str, max_size, "%.2f MFLOP/s", flopsd/1000000);

  return str;
}

void init_topology_struct(struct topology* topo, struct cache* cach) {
  topo->total_cores = 0;
  topo->cach = cach;
#if defined(ARCH_X86) || defined(ARCH_PPC)
  topo->physical_cores = 0;
  topo->logical_cores = 0;
  topo->smt_supported = 0;
  topo->sockets = 0;
#ifdef ARCH_X86
  topo->smt_available = 0;
  topo->apic = emalloc(sizeof(struct apic));
#endif
#endif
}

void init_cache_struct(struct cache* cach) {
  cach->L1i = emalloc(sizeof(struct cach));
  cach->L1d = emalloc(sizeof(struct cach));
  cach->L2 = emalloc(sizeof(struct cach));
  cach->L3 = emalloc(sizeof(struct cach));

  cach->cach_arr = emalloc(sizeof(struct cach*) * 4);
  cach->cach_arr[0] = cach->L1i;
  cach->cach_arr[1] = cach->L1d;
  cach->cach_arr[2] = cach->L2;
  cach->cach_arr[3] = cach->L3;

  cach->max_cache_level = 0;
  cach->L1i->exists = false;
  cach->L1d->exists = false;
  cach->L2->exists = false;
  cach->L3->exists = false;
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
