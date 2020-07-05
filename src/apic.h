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
};

bool get_topology_from_apic(uint32_t cpuid_max_levels, struct topology** topo);

#endif
