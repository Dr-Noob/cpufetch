#ifndef __SOCS__
#define __SOCS__

#include "soc.h"

// List of supported SOCs
enum {
  // SIFIVE
  SOC_SIFIVE_U740,
  // STARFIVE
  SOC_STARFIVE_VF2,
  // UNKNOWN
  SOC_MODEL_UNKNOWN
};

inline static VENDOR get_soc_vendor_from_soc(SOC soc) {
  if(soc >= SOC_SIFIVE_U740 && soc <= SOC_SIFIVE_U740) return SOC_VENDOR_SIFIVE;
  if(soc >= SOC_STARFIVE_VF2 && soc <= SOC_STARFIVE_VF2) return SOC_VENDOR_STARFIVE;
  return SOC_VENDOR_UNKNOWN;
}

#endif
