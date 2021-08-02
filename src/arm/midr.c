#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#ifdef __linux__
  #include <sys/auxv.h>
  #include <asm/hwcap.h>
#elif defined __APPLE__ || __MACH__
  #include "sysctl.h"
  #ifndef CPUFAMILY_ARM_FIRESTORM_ICESTORM
    #define CPUFAMILY_ARM_FIRESTORM_ICESTORM 0x1B588BB3
    // From arch/arm64/include/asm/cputype.h
    #define MIDR_APPLE_M1_ICESTORM  0x610F0220
    #define MIDR_APPLE_M1_FIRESTORM 0x610F0230
  #endif
#endif

#include "../common/global.h"
#include "udev.h"
#include "midr.h"
#include "uarch.h"
#include "soc.h"

#define STRING_UNKNOWN    "Unknown"

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
  freq->max = get_max_freq_from_file(core, false);

  return freq;
}

struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach, uint32_t* midr_array, int socket_idx, int ncores) {
  struct topology* topo = malloc(sizeof(struct topology));

  topo->cach = cach;
  topo->total_cores = 0;
  
  int sockets_seen = 0;
  int first_core_idx = 0;
  int currrent_core_idx = 0;
  int cores_in_socket = 0;
  
  while(socket_idx + 1 > sockets_seen) {
    if(midr_array[first_core_idx] == midr_array[currrent_core_idx] && currrent_core_idx < ncores) {
      currrent_core_idx++;
      cores_in_socket++;
    }
    else {
      topo->total_cores = cores_in_socket;  
      cores_in_socket = 0;
      first_core_idx = currrent_core_idx;
      sockets_seen++;
    }
  }
  
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

void init_cpu_info(struct cpuInfo* cpu) {
  cpu->next_cpu = NULL;
}

// We assume all cpus share the same hardware
// capabilities but I'm not sure it is always
// true...
// ARM32 https://elixir.bootlin.com/linux/latest/source/arch/arm/include/uapi/asm/hwcap.h
// ARM64 https://elixir.bootlin.com/linux/latest/source/arch/arm64/include/uapi/asm/hwcap.h
struct features* get_features_info() {
  struct features* feat = malloc(sizeof(struct features));
  bool *ptr = &(feat->AES);
  for(uint32_t i = 0; i < sizeof(struct features)/sizeof(bool); i++, ptr++) {
    *ptr = false;
  }

  #ifdef __linux__  
    errno = 0;
    long hwcaps = getauxval(AT_HWCAP);
  
    if(errno == ENOENT) {
      printWarn("Unable to retrieve AT_HWCAP using getauxval");    
    }
    #ifdef __aarch64__
      else {
        feat->AES = hwcaps & HWCAP_AES;
        feat->CRC32 = hwcaps & HWCAP_CRC32;
        feat->SHA1 = hwcaps & HWCAP_SHA1;
        feat->SHA2 = hwcaps & HWCAP_SHA2;
        feat->NEON = hwcaps & HWCAP_ASIMD;
      }
    #else
      else {
        feat->NEON = hwcaps & HWCAP_NEON;
      }

      hwcaps = getauxval(AT_HWCAP2);
      if(errno == ENOENT) {
        printWarn("Unable to retrieve AT_HWCAP2 using getauxval");
      }
      else {
        feat->AES = hwcaps & HWCAP2_AES;
        feat->CRC32 = hwcaps & HWCAP2_CRC32;
        feat->SHA1 = hwcaps & HWCAP2_SHA1;
        feat->SHA2 = hwcaps & HWCAP2_SHA2;
      }
    #endif // ifdef __aarch64__
  #elif defined __APPLE__ || __MACH__
    // Must be M1
    feat->AES = true;
    feat->CRC32 = true;
    feat->SHA1 = true;
    feat->SHA2 = true;
    feat->NEON = true;
  #endif

  return feat;
}

struct cpuInfo* get_cpu_info_linux(struct cpuInfo* cpu) {
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
    
    freq_array[i] = get_max_freq_from_file(i, false);
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
      init_cpu_info(ptr);

      tmp_midr_idx = midr_idx;
      while(cores_are_equal(midr_idx, tmp_midr_idx, midr_array, freq_array)) tmp_midr_idx++;
      midr_idx = tmp_midr_idx;
    }    
    
    ptr->midr = midr_array[midr_idx];
    ptr->arch = get_uarch_from_midr(ptr->midr, ptr);
    
    ptr->feat = get_features_info();
    ptr->freq = get_frequency_info(midr_idx);
    ptr->cach = get_cache_info(ptr);
    ptr->topo = get_topology_info(ptr, ptr->cach, midr_array, i, ncores);
  }
  
  cpu->num_cpus = sockets;
  cpu->hv = malloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  cpu->soc = get_soc();

  return cpu;
}

void fill_cpu_info_firestorm_icestorm(struct cpuInfo* cpu) {
  // 1. Fill ICESTORM
  struct cpuInfo* ice = cpu;

  ice->midr = MIDR_APPLE_M1_ICESTORM;
  ice->arch = get_uarch_from_midr(ice->midr, ice);
  ice->cach = get_cache_info(ice);
  ice->feat = get_features_info();
  ice->topo = malloc(sizeof(struct topology));
  ice->topo->cach = ice->cach;
  ice->topo->total_cores = 4;
  ice->freq = malloc(sizeof(struct frequency));
  ice->freq->base = UNKNOWN_FREQ;
  ice->freq->max = 2064;
  ice->hv = malloc(sizeof(struct hypervisor));
  ice->hv->present = false;
  ice->next_cpu = malloc(sizeof(struct cpuInfo));

  // 2. Fill FIRESTORM
  struct cpuInfo* fire = ice->next_cpu;
  fire->midr = MIDR_APPLE_M1_FIRESTORM;
  fire->arch = get_uarch_from_midr(fire->midr, fire);
  fire->cach = get_cache_info(fire);
  fire->feat = get_features_info();
  fire->topo = malloc(sizeof(struct topology));
  fire->topo->cach = fire->cach;
  fire->topo->total_cores = 4;
  fire->freq = malloc(sizeof(struct frequency));
  fire->freq->base = UNKNOWN_FREQ;
  fire->freq->max = 3200;
  fire->hv = malloc(sizeof(struct hypervisor));
  fire->hv->present = false;
  fire->next_cpu = NULL;
}

struct cpuInfo* get_cpu_info_mach(struct cpuInfo* cpu) {
  uint32_t cpu_family = get_sys_info_by_name("hw.cpufamily");

  // Manually fill the cpuInfo assuming that the CPU
  // is a ARM_FIRESTORM_ICESTORM (Apple M1)
  if(cpu_family == CPUFAMILY_ARM_FIRESTORM_ICESTORM) {
    cpu->num_cpus = 2;
    cpu->soc = get_soc();
    fill_cpu_info_firestorm_icestorm(cpu);
  }
  else {
    printBug("Found invalid cpu_family: 0x%.8X", cpu_family);
    return NULL;
  }

  return cpu;
}

struct cpuInfo* get_cpu_info() {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  init_cpu_info(cpu);

  #ifdef __linux__
    return get_cpu_info_linux(cpu);
  #elif defined __APPLE__ || __MACH__
    return get_cpu_info_mach(cpu);
  #endif
}

char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket) {
  uint32_t size = 3+7+1;
  char*  string = malloc(sizeof(char)*size);
  snprintf(string, size, "%d cores", topo->total_cores);

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
  if(cpu->feat->NEON) flops = flops * 4;
  
  if(flops >= (double)1000000000000.0)
    snprintf(string,size,"%.2f TFLOP/s",flops/1000000000000);
  else if(flops >= 1000000000.0)
    snprintf(string,size,"%.2f GFLOP/s",flops/1000000000);
  else
    snprintf(string,size,"%.2f MFLOP/s",flops/1000000);

  return string;
}

char* get_str_features(struct cpuInfo* cpu) {
  struct features* feat = cpu->feat;  
  char* string = malloc(sizeof(char) * 25);
  uint32_t len = 0;
  
  if(feat->NEON) {
    strcat(string, "NEON,");
    len += 5;
  }
  if(feat->SHA1) {
    strcat(string, "SHA1,");
    len += 5;
  }
  if(feat->SHA2) {
    strcat(string, "SHA2,");
    len += 5;
  }
  if(feat->AES) {
    strcat(string, "AES,");
    len += 4;
  }
  if(feat->CRC32) {
    strcat(string, "CRC32,");
    len += 6;
  }
  
  if(len > 0) {
    string[len-1] = '\0';
    return string;
  }
  else  
    return NULL;    
}

void print_debug(struct cpuInfo* cpu) {
  int ncores = get_ncores_from_cpuinfo();
  bool success = false;
  
  for(int i=0; i < ncores; i++) {
    printf("[Core %d] ", i);
    long freq = get_max_freq_from_file(i, false);
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
      printf("%ld MHz\n", get_max_freq_from_file(0, false));
    }
    else {
      printf("%ld MHz\n", freq);    
    }
  }    
}

void free_topo_struct(struct topology* topo) {
  free(topo);
}
