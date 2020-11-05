#ifndef __APIC__
#define __APIC__

#include <stdbool.h>
#include "cpuid.h"

struct apic {
  uint32_t pkg_mask;
  uint32_t pkg_mask_shift;
  uint32_t core_mask;
  uint32_t smt_mask_width;
  uint32_t smt_mask;
  uint32_t* cache_select_mask;
  uint32_t* cache_id_apic;
};

bool get_topology_from_apic(struct cpuInfo* cpu, struct topology* topo);
uint32_t is_smt_enabled_amd(struct topology* topo);

#endif
