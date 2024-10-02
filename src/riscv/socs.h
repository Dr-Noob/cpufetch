#ifndef __SOCS__
#define __SOCS__

#include "soc.h"

// List of supported SOCs
enum {
  // SIFIVE
  SOC_SIFIVE_U740,
  // STARFIVE
  SOC_STARFIVE_VF2,
  // ALLWINNER
  SOC_ALLWINNER_D1H,
  // SIPEED
  SOC_SIPEED_LICHEEPI4A,
  // SPACEMIT
  SOC_SPACEMIT_K1,
  // UNKNOWN
  SOC_MODEL_UNKNOWN
};

inline static VENDOR get_soc_vendor_from_soc(SOC soc) {
  if(soc >= SOC_SIFIVE_U740 && soc <= SOC_SIFIVE_U740) return SOC_VENDOR_SIFIVE;
  if(soc >= SOC_STARFIVE_VF2 && soc <= SOC_STARFIVE_VF2) return SOC_VENDOR_STARFIVE;
  if(soc >= SOC_ALLWINNER_D1H && soc <= SOC_ALLWINNER_D1H) return SOC_VENDOR_ALLWINNER;
  if(soc >= SOC_SIPEED_LICHEEPI4A && soc <= SOC_SIPEED_LICHEEPI4A) return SOC_VENDOR_SIPEED;
  if(soc >= SOC_SPACEMIT_K1 && soc <= SOC_SPACEMIT_K1) return SOC_VENDOR_SPACEMIT;
  return SOC_VENDOR_UNKNOWN;
}

#endif
