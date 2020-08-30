#ifdef _WIN32
#include <windows.h>
#else
#define _GNU_SOURCE
#include <sched.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "apic.h"
#include "cpuid_asm.h"
#include "global.h"

/*
 * bit_scan_reverse and create_mask code taken from:
 * https://software.intel.com/content/www/us/en/develop/articles/intel-64-architecture-processor-topology-enumeration.html
 */
unsigned char bit_scan_reverse(uint32_t* index, uint64_t mask) {
  for(uint64_t i = (8 * sizeof(uint64_t)); i > 0; i--) {
    if((mask & (1LL << (i-1))) != 0) {
      *index = (uint64_t) (i-1);
      break;
    }
  }
  return (unsigned char) (mask != 0);
}

uint32_t create_mask(uint32_t num_entries, uint32_t *mask_width) {
  uint32_t i = 0;
  uint64_t k = 0;

  // NearestPo2(numEntries) is the nearest power of 2 integer that is not less than numEntries
  // The most significant bit of (numEntries * 2 -1) matches the above definition

  k = (uint64_t)(num_entries) * 2 -1;

  if (bit_scan_reverse(&i, k) == 0) {
    if (mask_width) *mask_width = 0;
    return 0;
  }

  if (mask_width) *mask_width = i;
  if (i == 31) return (uint32_t ) -1;

  return (1 << i) -1;
}

uint32_t get_apic_id(bool x2apic_id) {
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  
  if(x2apic_id) {
    eax = 0x0000000B;
    cpuid(&eax, &ebx, &ecx, &edx);
    return edx;
  }
  else {
    eax = 0x00000001;
    cpuid(&eax, &ebx, &ecx, &edx);
    return (ebx >> 24);
  }
}

bool bind_to_cpu(int cpu_id) {
  #ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    DWORD_PTR processAffinityMask = 1 << cpu_id;
    return SetProcessAffinityMask(process, processAffinityMask);
  #else    
    cpu_set_t currentCPU;
    CPU_ZERO(&currentCPU);
    CPU_SET(cpu_id, &currentCPU);
    if (sched_setaffinity (0, sizeof(currentCPU), &currentCPU) == -1) {
      perror("sched_setaffinity");
      return false;
    }
    return true;
  #endif  
}

bool fill_topo_masks_apic(struct topology** topo) {
  uint32_t eax = 0x00000001;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  uint32_t core_plus_smt_id_max_cnt;
  uint32_t core_id_max_cnt;
  uint32_t smt_id_per_core_max_cnt;
  
  cpuid(&eax, &ebx, &ecx, &edx);
  
  core_plus_smt_id_max_cnt = (ebx >> 16) & 0xFF;
  
  eax = 0x00000004;
  ecx = 0;
  cpuid(&eax, &ebx, &ecx, &edx);
  
  core_id_max_cnt = (eax >> 26) + 1;
  smt_id_per_core_max_cnt = core_plus_smt_id_max_cnt / core_id_max_cnt; 
            
  (*topo)->apic->smt_mask = create_mask(smt_id_per_core_max_cnt, &((*topo)->apic->smt_mask_width));    
  (*topo)->apic->core_mask = create_mask(core_id_max_cnt,&((*topo)->apic->pkg_mask_shift));
  (*topo)->apic->pkg_mask_shift += (*topo)->apic->smt_mask_width;
  (*topo)->apic->core_mask <<= (*topo)->apic->smt_mask_width;
  (*topo)->apic->pkg_mask = (-1) ^ ((*topo)->apic->core_mask | (*topo)->apic->smt_mask);
  
  return true;
}

bool fill_topo_masks_x2apic(struct topology** topo) {
  int32_t level_type;
  int32_t level_shift;
  
  int32_t coreplus_smt_mask = 0;
  bool level2 = false;
  bool level1 = false;
  
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  uint32_t i = 0;
  
  while(true) {
    eax = 0x0000000B;
    ecx = i;
    cpuid(&eax, &ebx, &ecx, &edx);
    if(ebx == 0) break;
    
    level_type = (ecx >> 8) & 0xFF;
    level_shift = eax & 0xFFF; 
    
    switch(level_type) {      
      case 1: // SMT
        (*topo)->apic->smt_mask = ~(0xFFFFFFFF << level_shift);
        (*topo)->apic->smt_mask_width = level_shift;
        (*topo)->smt_supported = ebx & 0xFFFF;
        level1 = true;
        break;
      case 2: // Core
        coreplus_smt_mask = ~(0xFFFFFFFF << level_shift);
        (*topo)->apic->pkg_mask_shift =  level_shift;
        (*topo)->apic->pkg_mask = (-1) ^ coreplus_smt_mask;
        level2 = true;
        break;
      default:
        printErr("Found invalid level when querying topology: %d", level_type);
        break;
    }
    
    i++; // sublevel to query
  }
  
  if (level1 && level2) {
    (*topo)->apic->core_mask = coreplus_smt_mask ^ (*topo)->apic->smt_mask;
  }
  else if (!level2 && level1) {
    (*topo)->apic->core_mask = 0;
    (*topo)->apic->pkg_mask_shift = (*topo)->apic->smt_mask_width;
    (*topo)->apic->pkg_mask =  (-1) ^ (*topo)->apic->smt_mask;
  }
  else {
    printErr("SMT level was not found when querying topology");
    return false;
  }

  return true;
}

bool arr_contains_value(uint32_t* arr, uint32_t value, uint32_t arr_size) {
  for(uint32_t i=0; i < arr_size; i++) {
    if(arr[i] == value) return true;    
  }
  return false;
}

uint32_t max_apic_id_size(uint32_t** cache_id_apic, struct topology** topo) {
  uint32_t max = 0;
  
  for(int i=0; i < (*topo)->cach->max_cache_level; i++) {
    for(int j=0; j < (*topo)->total_cores; j++) {          
      if(cache_id_apic[j][i] > max) max = cache_id_apic[j][i];
    }
  }
  
  max++;
  if(max > (*topo)->total_cores) return max;
  return (*topo)->total_cores;
}

bool build_topo_from_apic(uint32_t* apic_pkg, uint32_t* apic_smt, uint32_t** cache_id_apic, struct topology** topo) {
  uint32_t size = max_apic_id_size(cache_id_apic, topo);
  uint32_t* sockets = malloc(sizeof(uint32_t) * size);
  uint32_t* smt = malloc(sizeof(uint32_t) * size);
  uint32_t* apic_id = malloc(sizeof(uint32_t) * size);
  uint32_t num_caches = 0;
  
  memset(sockets, 0, sizeof(uint32_t) * size);
  memset(smt, 0, sizeof(uint32_t) * size);  
  memset(apic_id, 0, sizeof(uint32_t) * size);  
  
  for(int i=0; i < (*topo)->total_cores; i++) {
    sockets[apic_pkg[i]] = 1;
    smt[apic_smt[i]] = 1;
  }
  for(int i=0; i < (*topo)->total_cores; i++) {
    if(sockets[i] != 0)
      (*topo)->sockets++;
    if(smt[i] != 0)
      (*topo)->smt_available++;
  }
  
  (*topo)->logical_cores = (*topo)->total_cores / (*topo)->sockets;
  (*topo)->physical_cores = (*topo)->logical_cores / (*topo)->smt_available;
  
  for(int i=0; i < (*topo)->cach->max_cache_level; i++) {
    num_caches = 0;
    memset(apic_id, 0, sizeof(uint32_t) * size);
    
    for(int c=0; c < (*topo)->total_cores; c++) {      
      apic_id[cache_id_apic[c][i]]++;
    }
    for(uint32_t c=0; c < size; c++) {      
      if(apic_id[c] > 0) num_caches++;
    }
    
    (*topo)->cach->cach_arr[i]->num_caches = num_caches;
  }
  
  free(sockets);
  free(smt);
  free(apic_id);
  
  return true;
}

bool get_cache_topology_from_apic(struct topology** topo) {  
  uint32_t eax = 0x00000004;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
     
  for(int i=0; i < (*topo)->cach->max_cache_level; i++) { 
    eax = 0x00000004;
    ecx = i;
    
    cpuid(&eax, &ebx, &ecx, &edx);
  
    uint32_t SMTMaxCntPerEachCache = ((eax >> 14) & 0x7FF) + 1;
    uint32_t EachCacheMaskWidth_targ_subleaf;
    (*topo)->apic->cache_select_mask[i] = create_mask(SMTMaxCntPerEachCache,&EachCacheMaskWidth_targ_subleaf);
  }
  
  return true;
}

bool get_topology_from_apic(struct cpuInfo* cpu, struct topology** topo) { 
  uint32_t apic_id;
  uint32_t* apic_pkg = malloc(sizeof(uint32_t) * (*topo)->total_cores);
  uint32_t* apic_core = malloc(sizeof(uint32_t) * (*topo)->total_cores);
  uint32_t* apic_smt = malloc(sizeof(uint32_t) * (*topo)->total_cores);
  uint32_t** cache_smt_id_apic = malloc(sizeof(uint32_t*) * (*topo)->total_cores);
  uint32_t** cache_id_apic = malloc(sizeof(uint32_t*) * (*topo)->total_cores);
  bool x2apic_id = cpu->maxLevels >= 0x0000000B;
  
  for(int i=0; i < (*topo)->total_cores; i++) {
    cache_smt_id_apic[i] = malloc(sizeof(uint32_t) * ((*topo)->cach->max_cache_level));
    cache_id_apic[i] = malloc(sizeof(uint32_t) * ((*topo)->cach->max_cache_level));
  }
  (*topo)->apic->cache_select_mask = malloc(sizeof(uint32_t) * ((*topo)->cach->max_cache_level));
  (*topo)->apic->cache_id_apic = malloc(sizeof(uint32_t) * ((*topo)->cach->max_cache_level));
  
  if(x2apic_id) {
    if(!fill_topo_masks_x2apic(topo))
      return false;
  }
  else {
    if(!fill_topo_masks_apic(topo))
      return false;    
  }
  
  get_cache_topology_from_apic(topo);  
  
  for(int i=0; i < (*topo)->total_cores; i++) {
    if(!bind_to_cpu(i)) {
      printErr("Failed binding to CPU %d", i);
      return false;
    }
    apic_id = get_apic_id(x2apic_id);
    
    apic_pkg[i] = (apic_id & (*topo)->apic->pkg_mask) >> (*topo)->apic->pkg_mask_shift;
    apic_core[i] = (apic_id & (*topo)->apic->core_mask) >> (*topo)->apic->smt_mask_width;
    apic_smt[i] = apic_id & (*topo)->apic->smt_mask;
    
    for(int c=0; c < (*topo)->cach->max_cache_level; c++) {
      cache_smt_id_apic[i][c] = apic_id & (*topo)->apic->cache_select_mask[c];
      cache_id_apic[i][c] = apic_id & (-1 ^ (*topo)->apic->cache_select_mask[c]);
    }
  }
  
  /* DEBUG
  for(int i=0; i < (*topo)->cach->max_cache_level; i++) {
    printf("[CACH %1d]", i);
    for(int j=0; j < (*topo)->total_cores; j++)
      printf("[%03d]", cache_id_apic[j][i]);
    printf("\n");
  }  
  for(int i=0; i < (*topo)->total_cores; i++)
    printf("[%2d] 0x%.8X\n", i, apic_pkg[i]);
  printf("\n");
  for(int i=0; i < (*topo)->total_cores; i++)
    printf("[%2d] 0x%.8X\n", i, apic_core[i]);
  printf("\n");
  for(int i=0; i < (*topo)->total_cores; i++)
    printf("[%2d] 0x%.8X\n", i, apic_smt[i]);*/
    
  
  bool ret = build_topo_from_apic(apic_pkg, apic_smt, cache_id_apic, topo);
  
  // Assumption: If we cant get smt_available, we assume it is equal to smt_supported...
  if (!x2apic_id) (*topo)->smt_supported = (*topo)->smt_available;
  
  //TODO: free
  
  return ret;
} 

uint32_t is_smt_enabled_amd(struct topology* topo) {
  uint32_t id;
  
  for(int i = 0; i < topo->total_cores; i++) {
    if(!bind_to_cpu(i)) {
      printErr("Failed binding to CPU %d", i);
      return false;
    }
    id = get_apic_id(false) & 1; // get the last bit
    if(id == 1) return 2; // We assume there isn't any AMD CPU with more than 2th per core. TODO: Fix
  }
  
  return 1;  
}
