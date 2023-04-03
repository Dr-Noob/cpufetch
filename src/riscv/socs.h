#ifndef __SOCS__
#define __SOCS__

#include "soc.h"

// List of supported SOCs
enum {
  // SIFIVE
  SOC_SIFIVE_U740,
  // UNKNOWN
  SOC_MODEL_UNKNOWN
};

inline static VENDOR get_soc_vendor_from_soc(SOC soc) {
  if(soc >= SOC_SIFIVE_U740 && soc <= SOC_SIFIVE_U740) return SOC_VENDOR_SIFIVE;
  return SOC_VENDOR_UNKNOWN;
}

#endif
