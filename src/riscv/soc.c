#include "soc.h"
#include "socs.h"
#include "udev.h"
#include "../common/global.h"

#include <string.h>

bool match_sifive(char* soc_name, struct system_on_chip* soc) {
  char* tmp = soc_name;

  // Dont know if it makes sense in RISC-V
  /*if((tmp = strstr(soc_name, "???")) == NULL)
    return false;*/

  //soc->vendor = ???

  SOC_START
  SOC_EQ(tmp, "fu740", "Freedom U740",  SOC_SIFIVE_U740, soc, 40)
  SOC_END
}

bool match_starfive(char* soc_name, struct system_on_chip* soc) {
  SOC_START
  SOC_EQ(soc_name, "jh7110#", "VisionFive 2",  SOC_STARFIVE_VF2, soc, 28) // https://blog.bitsofnetworks.org/benchmarking-risc-v-visionfive-2-vs-the-world.html
  SOC_EQ(soc_name, "jh7110",  "VisionFive 2",  SOC_STARFIVE_VF2, soc, 28)
  SOC_END
}

bool match_allwinner(char* soc_name, struct system_on_chip* soc) {
  SOC_START
  SOC_EQ(soc_name, "sun20i-d1", "D1-H", SOC_ALLWINNER_D1H, soc, 22)
  SOC_END
}

bool match_sipeed(char* soc_name, struct system_on_chip* soc) {
  SOC_START
  SOC_EQ(soc_name, "light", "Lichee Pi 4A", SOC_SIPEED_LICHEEPI4A, soc, 12) // https://github.com/Dr-Noob/cpufetch/issues/200, https://sipeed.com/licheepi4a
  SOC_END
}

struct system_on_chip* parse_soc_from_string(struct system_on_chip* soc) {
  char* raw_name = soc->raw_name;

  if(match_starfive(raw_name, soc))
    return soc;

  if(match_allwinner(raw_name, soc))
    return soc;

  if(match_sifive(raw_name, soc))
    return soc;

  match_sipeed(raw_name, soc);
  return soc;
}

struct system_on_chip* guess_soc_from_devtree(struct system_on_chip* soc) {
  char* tmp = get_hardware_from_devtree();

  if(tmp != NULL) {
    soc->raw_name = tmp;
    return parse_soc_from_string(soc);
  }

  return soc;
}

struct system_on_chip* get_soc(struct cpuInfo* cpu) {
  struct system_on_chip* soc = emalloc(sizeof(struct system_on_chip));
  soc->raw_name = NULL;
  soc->vendor = SOC_VENDOR_UNKNOWN;
  soc->model = SOC_MODEL_UNKNOWN;
  soc->process = UNKNOWN;

  soc = guess_soc_from_devtree(soc);
  if(soc->vendor == SOC_VENDOR_UNKNOWN) {
    if(soc->raw_name != NULL) {
      printWarn("SoC detection failed using device tree: Found '%s' string", soc->raw_name);
    }
    else {
      printWarn("SoC detection failed using device tree");
    }
  }

  if(soc->model == SOC_MODEL_UNKNOWN) {
    // raw_name might not be NULL, but if we were unable to find
    // the exact SoC, just print "Unkwnown"
    soc->raw_name = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }

  return soc;
}
