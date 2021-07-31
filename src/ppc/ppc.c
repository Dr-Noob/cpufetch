#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "ppc.h"
#include "uarch.h"
#include "udev.h"
#include "../common/udev.h"

#define STRING_UNKNOWN    "Unknown"

void init_topology_struct(struct topology* topo, struct cache* cach) {
  topo->total_cores = 0;
  topo->physical_cores = 0;
  topo->logical_cores = 0;
  topo->smt_supported = 0;
  topo->sockets = 0;
  topo->cach = cach;
}

void init_cache_struct(struct cache* cach) {
  cach->L1i = malloc(sizeof(struct cach));
  cach->L1d = malloc(sizeof(struct cach));
  cach->L2 = malloc(sizeof(struct cach));
  cach->L3 = malloc(sizeof(struct cach));
  
  cach->cach_arr = malloc(sizeof(struct cach*) * 4);
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

struct cache* get_cache_info(struct cpuInfo* cpu) {
  struct cache* cach = malloc(sizeof(struct cache));
  init_cache_struct(cach);

  cach->L1i->size = get_l1i_cache_size(0);
  cach->L1d->size = get_l1d_cache_size(0);
  cach->L2->size = get_l2_cache_size(0);
  cach->L3->size = get_l3_cache_size(0);

  if(cach->L1i->size > 0) {
    cach->L1i->exists = true;
    cach->L1i->num_caches = cpu->topo->physical_cores * cpu->topo->sockets;
    cach->max_cache_level++;
  }
  if(cach->L1d->size > 0) {
    cach->L1d->exists = true;
    cach->L1d->num_caches = cpu->topo->physical_cores * cpu->topo->sockets;
    cach->max_cache_level++;
  }
  if(cach->L2->size > 0) {
    cach->L2->exists = true;
    cach->L2->num_caches = cpu->topo->physical_cores * cpu->topo->sockets;
    cach->max_cache_level++;
  }
  if(cach->L3->size > 0) {
    cach->L3->exists = true;
    cach->L3->num_caches = cpu->topo->physical_cores * cpu->topo->sockets; // does not have to be true!
    cach->max_cache_level++;
  }

  return cach;
}

struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach) {
  struct topology* topo = malloc(sizeof(struct topology));
  init_topology_struct(topo, cach);

  // 1. Total cores detection
  if((topo->total_cores = sysconf(_SC_NPROCESSORS_ONLN)) == -1) {
    perror("sysconf");
    return NULL;
  }

  // To find physical cores, we use topo->total_cores and core_ids
  // To find number of sockets, we use package_ids
  int* core_ids = malloc(sizeof(int) * topo->total_cores);
  int* package_ids = malloc(sizeof(int) * topo->total_cores);

  fill_core_ids_from_sys(core_ids, topo->total_cores);
  fill_package_ids_from_sys(package_ids, topo->total_cores);

  // 2. Socket detection
  int *package_ids_count = malloc(sizeof(int) * topo->total_cores);
  for(int i=0; i < topo->total_cores; i++) {
    package_ids_count[i] = 0;
  }
  for(int i=0; i < topo->total_cores; i++) {
    package_ids_count[package_ids[i]]++;
  }
  for(int i=0; i < topo->total_cores; i++) {
    if(package_ids_count[i] != 0) {
      topo->sockets++;
    }
  }

  // 3. Physical cores detection
  int *core_ids_unified = malloc(sizeof(int) * topo->total_cores);
  for(int i=0; i < topo->total_cores; i++) {
    core_ids_unified[i] = -1;
  }
  bool found = false;
  for(int i=0; i < topo->total_cores; i++) {
    for(int j=0; j < topo->total_cores && !found; j++) {
      if(core_ids_unified[j] == core_ids[i]) found = true;
    }
    if(!found) {
      core_ids_unified[topo->physical_cores] = core_ids[i];
      topo->physical_cores++;
    }
    found = false;
  }

  topo->physical_cores = topo->physical_cores / topo->sockets; // only count cores on one socket
  topo->logical_cores = topo->total_cores / topo->sockets;     // only count threads on one socket
  topo->smt_supported = topo->logical_cores / topo->physical_cores;

  free(core_ids);
  free(package_ids);
  free(package_ids_count);
  free(core_ids_unified);

  return topo;
}

static inline uint32_t mfpvr() {
    uint32_t pvr;

    asm ("mfpvr %0"
         : "=r"(pvr));
    return pvr;
}

struct uarch* get_cpu_uarch() {
  uint32_t pvr = mfpvr();
  return get_uarch_from_pvr(pvr);
}

struct frequency* get_frequency_info() {
  struct frequency* freq = malloc(sizeof(struct frequency));

  freq->max = get_max_freq_from_file(0, false);
  freq->base = get_min_freq_from_file(0, false);

  return freq;
}

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  struct features* feat = malloc(sizeof(struct features));
  cpu->feat = feat;

  bool *ptr = &(feat->AES);
  for(uint32_t i = 0; i < sizeof(struct features)/sizeof(bool); i++, ptr++) {
    *ptr = false;
  }

  cpu->cpu_name = malloc(sizeof(char) * strlen(STRING_UNKNOWN) + 1);
  snprintf(cpu->cpu_name, strlen(STRING_UNKNOWN) + 1, STRING_UNKNOWN);

  cpu->arch = get_cpu_uarch();
  cpu->freq = get_frequency_info();
  cpu->topo = get_topology_info(cpu, cpu->cach);
  cpu->cach = get_cache_info(cpu);

  feat->altivec = has_altivec(cpu->arch);

  if(cpu->cach == NULL || cpu->topo == NULL) {
    return NULL;
  }
  return cpu;
}

char* get_str_altivec(struct cpuInfo* cpu) {
  char* string = calloc(4, sizeof(char));

  if(cpu->feat->altivec) strcpy(string, "Yes");
  else strcpy(string, "No");

  return string;
}

char* get_str_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq) {
  /*
   * Not sure about this
   * PP(SP) = N_CORES * FREQUENCY * 4(If altivec)
   */

  //7 for GFLOP/s and 6 for digits,eg 412.14
  uint32_t size = 7+6+1+1;
  assert(strlen(STRING_UNKNOWN)+1 <= size);
  char* string = malloc(sizeof(char)*size);

  //First check we have consistent data
  if(freq == UNKNOWN_FREQ) {
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
    return string;
  }

  struct features* feat = cpu->feat;
  double flops = topo->physical_cores * topo->sockets * (freq*1000000);
  if(feat->altivec) flops = flops*4;

  if(flops >= (double)1000000000000.0)
    snprintf(string,size,"%.2f TFLOP/s",flops/1000000000000);
  else if(flops >= 1000000000.0)
    snprintf(string,size,"%.2f GFLOP/s",flops/1000000000);
  else
    snprintf(string,size,"%.2f MFLOP/s",flops/1000000);

  return string;
}

char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket) {
  char* string;
  if(topo->smt_supported > 1) {
    uint32_t size = 3+3+17+1;
    string = malloc(sizeof(char)*size);
    if(dual_socket)
      snprintf(string, size, "%d cores (%d threads)", topo->physical_cores * topo->sockets, topo->logical_cores * topo->sockets);
    else
      snprintf(string, size, "%d cores (%d threads)",topo->physical_cores,topo->logical_cores);
  }
  else {
    uint32_t size = 3+7+1;
    string = malloc(sizeof(char)*size);
    if(dual_socket)
      snprintf(string, size, "%d cores",topo->physical_cores * topo->sockets);
    else
      snprintf(string, size, "%d cores",topo->physical_cores);
  }
  return string;
}


void print_debug(struct cpuInfo* cpu) {
  printf("TODO\n");
}
