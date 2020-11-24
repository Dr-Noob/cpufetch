#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "../common/global.h"
#include "udev.h"
#include "midr.h"
#include "uarch.h"
#include "soc.h"

#define STRING_UNKNOWN    "Unknown"

void init_topology_struct(struct topology* topo, struct cache* cach) {
  topo->total_cores = 0;
  topo->physical_cores = 0;
  topo->logical_cores = 0;
  topo->smt_available = 0;
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

  cach->max_cache_level = 2;
  for(int i=0; i < cach->max_cache_level + 1; i++) {
    cach->cach_arr[i]->exists = true;
    cach->cach_arr[i]->num_caches = 1;
    cach->cach_arr[i]->size = 0;
  }

  return cach;
}

struct frequency* get_frequency_info(uint32_t core) {
  struct frequency* freq = malloc(sizeof(struct frequency));

  freq->base = UNKNOWN_FREQ;
  freq->max = get_max_freq_from_file(core);

  return freq;
}

struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach) {
  struct topology* topo = malloc(sizeof(struct topology));
  init_topology_struct(topo, cach);

  topo->total_cores = get_ncores_from_cpuinfo();
  topo->physical_cores = topo->total_cores;
  topo->logical_cores = topo->total_cores;
  topo->smt_available = 1;
  topo->smt_supported = 0;
  topo->sockets = 1;

  return topo;
}

bool cores_are_equal(int c1pos, int c2pos, uint32_t* midr_array, int32_t* freq_array) {
  return midr_array[c1pos] == midr_array[c2pos] && freq_array[c1pos] == freq_array[c2pos];
}
    
uint32_t fill_ids_from_midr(uint32_t* midr_array, int32_t* freq_array, uint32_t* ids_array, int len) {
  uint32_t latest_id = 0;
  bool found;
  ids_array[0] = latest_id;

  for (int i = 1; i < len; i++) {
    int j = 0;
    found = false;

    for (j = 0; j < len && !found; j++) {
      if (i != j && cores_are_equal(i, j, midr_array, freq_array)) {
        if(j > i) {
          latest_id++;
          ids_array[i] = latest_id;
        }
        else {
          ids_array[i] = ids_array[j];
        }
        found = true;
      }
    }
    if(!found) {
      latest_id++;
      ids_array[i] = latest_id;
    }
  }

  return latest_id+1;
}

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  cpu->next_cpu = NULL;

  int ncores = get_ncores_from_cpuinfo();
  bool success = false;
  int32_t* freq_array = malloc(sizeof(uint32_t) * ncores);
  uint32_t* midr_array = malloc(sizeof(uint32_t) * ncores);  
  uint32_t* ids_array = malloc(sizeof(uint32_t) * ncores);

  for(int i=0; i < ncores; i++) {
    midr_array[i] = get_midr_from_cpuinfo(i, &success);
    
    if(!success) {
      printWarn("Unable to fetch MIDR for core %d. This is probably because the core is offline", i);
      midr_array[i] = midr_array[0];
    }
    
    freq_array[i] = get_max_freq_from_file(i);
    if(freq_array[i] == UNKNOWN_FREQ) {
      printWarn("Unable to fetch max frequency for core %d. This is probably because the core is offline", i);
      freq_array[i] = freq_array[0];
    }    
  }
  uint32_t sockets = fill_ids_from_midr(midr_array, freq_array, ids_array, ncores);

  struct cpuInfo* ptr = cpu;
  int midr_idx = 0;
  int tmp_midr_idx = 0;
  for(uint32_t i=0; i < sockets; i++) {
    if(i > 0) {
      ptr->next_cpu = malloc(sizeof(struct cpuInfo));
      ptr = ptr->next_cpu;

      tmp_midr_idx = midr_idx;
      while(cores_are_equal(midr_idx, tmp_midr_idx, midr_array, freq_array)) tmp_midr_idx++;
      midr_idx = tmp_midr_idx;
    }
    ptr->next_cpu = NULL;
    
    ptr->midr = midr_array[midr_idx];
    ptr->arch = get_uarch_from_midr(ptr->midr, ptr);
    
    ptr->freq = get_frequency_info(i); // TODO: wrong!
    ptr->cach = get_cache_info(ptr);
    ptr->topo = get_topology_info(ptr, ptr->cach);
  }
  
  cpu->num_cpus = sockets;
  cpu->hv = malloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  cpu->soc = get_soc();  

  return cpu;
}

char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket) {
  uint32_t size = 3+7+1;
  char*  string = malloc(sizeof(char)*size);
  snprintf(string, size, "%d cores", topo->physical_cores);

  return string;
}

char* get_str_peak_performance(struct cpuInfo* cpu) { 
  //7 for GFLOP/s and 6 for digits,eg 412.14
  uint32_t size = 7+6+1+1;
  assert(strlen(STRING_UNKNOWN)+1 <= size);
  char* string = malloc(sizeof(char)*size);
  struct cpuInfo* ptr = cpu;
  
  //First check we have consistent data
  for(int i=0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
    if(get_freq(ptr->freq) == UNKNOWN_FREQ) {
      snprintf(string, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
      return string;
    }
  }

  double flops = 0.0;
  
  ptr = cpu;
  for(int i=0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
    flops += ptr->topo->total_cores * (get_freq(ptr->freq) * 1000000);
  }
  
  if(flops >= (double)1000000000000.0)
    snprintf(string,size,"%.2f TFLOP/s",flops/1000000000000);
  else if(flops >= 1000000000.0)
    snprintf(string,size,"%.2f GFLOP/s",flops/1000000000);
  else
    snprintf(string,size,"%.2f MFLOP/s",flops/1000000);

  return string;
}

void print_debug(struct cpuInfo* cpu) {
  int ncores = get_ncores_from_cpuinfo();
  bool success = false;
  
  for(int i=0; i < ncores; i++) {
    printf("[Core %d] ", i);
    long freq = get_max_freq_from_file(i);
    uint32_t midr = get_midr_from_cpuinfo(i, &success);
    if(!success) {
      printWarn("Unable to fetch MIDR for core %d. This is probably because the core is offline", i);
      printf("0x%.8X ", get_midr_from_cpuinfo(0, &success));
    }
    else {
      printf("0x%.8X ", midr);    
    }
    if(freq == UNKNOWN_FREQ) {
      printWarn("Unable to fetch max frequency for core %d. This is probably because the core is offline", i);
      printf("%ld MHz\n", get_max_freq_from_file(0));
    }
    else {
      printf("%ld MHz\n", freq);    
    }
  }    
}

void free_topo_struct(struct topology* topo) {
  free(topo);
}
