#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "midr.h"

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));

  cpu->cpu_vendor = CPU_VENDOR_UNKNOWN;
  cpu->cpu_name = malloc(sizeof(char) * CPU_NAME_MAX_LENGTH);
  strcpy(cpu->cpu_name, "Unknown");
  cpu->arch = NULL;
  cpu->hv = malloc(sizeof(struct hypervisor));
  cpu->hv->present = false;

  return cpu;
}

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
  return cach;
}

struct frequency* get_frequency_info(struct cpuInfo* cpu) { 
  struct frequency* freq = malloc(sizeof(struct frequency));
  
  freq->base = UNKNOWN_FREQ;
  freq->max = UNKNOWN_FREQ;  
  
  return freq;    
}

struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach) {
  struct topology* topo = malloc(sizeof(struct topology));  
  init_topology_struct(topo, cach);
  return topo;
}

VENDOR get_cpu_vendor(struct cpuInfo* cpu) {
  return cpu->cpu_vendor;
}
uint32_t get_nsockets(struct topology* topo) { return 0; }
int64_t get_freq(struct frequency* freq) { return 0; }

char* get_str_cpu_name(struct cpuInfo* cpu) {
  return cpu->cpu_name;    
}
char* get_str_ncores(struct cpuInfo* cpu){ return NULL; }
char* get_str_avx(struct cpuInfo* cpu){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "No"); return tmp; }
char* get_str_sse(struct cpuInfo* cpu){ return NULL; }
char* get_str_fma(struct cpuInfo* cpu){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "No"); return tmp; }
char* get_str_aes(struct cpuInfo* cpu){ return NULL; }
char* get_str_sha(struct cpuInfo* cpu){ return NULL; }

char* get_str_l1i(struct cache* cach){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "0 KB"); return tmp; }
char* get_str_l1d(struct cache* cach){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "0 KB"); return tmp; }
char* get_str_l2(struct cache* cach){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "0 KB"); return tmp; }
char* get_str_l3(struct cache* cach){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "0 KB"); return tmp; }

char* get_str_freq(struct frequency* freq){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "0 MHz"); return tmp; }

char* get_str_sockets(struct topology* topo){ return NULL; }
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket){ char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "0 cores"); return tmp; }

char* get_str_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq) { char* tmp = malloc(sizeof(char) * 10); strcpy(tmp, "0 MFLOP/s"); return tmp; }

void free_cache_struct(struct cache* cach){ }
void free_topo_struct(struct topology* topo){ }
void free_freq_struct(struct frequency* freq){ }
void free_cpuinfo_struct(struct cpuInfo* cpu){ }

void debug_cpu_info(struct cpuInfo* cpu){  }
void debug_cache(struct cache* cach){  }
void debug_frequency(struct frequency* freq){  }
