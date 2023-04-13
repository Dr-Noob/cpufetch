#include "soc.h"
#include "socs.h"
#include "udev.h"
#include "../common/global.h"

#include <string.h>

VENDOR get_soc_vendor(struct system_on_chip* soc) {
  return soc->soc_vendor;
}

char* get_str_process(struct system_on_chip* soc) {
  char* str;

  if(soc->process == UNKNOWN) {
    str = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(str, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  else {
    str = emalloc(sizeof(char) * 5);
    memset(str, 0, sizeof(char) * 5);
    snprintf(str, 5, "%dnm", soc->process);
  }
  return str;
}

char* get_soc_name(struct system_on_chip* soc) {
  if(soc->soc_model == SOC_MODEL_UNKNOWN)
    return soc->raw_name;
  return soc->soc_name;
}

static char* soc_trademark_string[] = {
  [SOC_VENDOR_SIFIVE] = "SiFive ",
  [SOC_VENDOR_STARFIVE] = "StarFive ",
  [SOC_VENDOR_ALLWINNER]  = "Allwinner "
};

void fill_soc(struct system_on_chip* soc, char* soc_name, SOC soc_model, int32_t process) {
  soc->soc_model = soc_model;
  soc->soc_vendor = get_soc_vendor_from_soc(soc_model);
  soc->process = process;
  int len = strlen(soc_name) + strlen(soc_trademark_string[soc->soc_vendor]) + 1;
  soc->soc_name = emalloc(sizeof(char) * len);
  memset(soc->soc_name, 0, sizeof(char) * len);
  sprintf(soc->soc_name, "%s%s", soc_trademark_string[soc->soc_vendor], soc_name);
}

bool match_soc(struct system_on_chip* soc, char* raw_name, char* expected_name, char* soc_name, SOC soc_model, int32_t process) {
  int len1 = strlen(raw_name);
  int len2 = strlen(expected_name);
  int len = min(len1, len2);

  if(strncmp(raw_name, expected_name, len) != 0) {
    return false;
  }
  else {
    fill_soc(soc, soc_name, soc_model, process);
    return true;
  }
}

#define SOC_START if (false) {}
#define SOC_EQ(raw_name, expected_name, soc_name, soc_model, soc, process) \
   else if (match_soc(soc, raw_name, expected_name, soc_name, soc_model, process)) return true;
#define SOC_END else { return false; }

bool match_sifive(char* soc_name, struct system_on_chip* soc) {
  char* tmp = soc_name;

  // Dont know if it makes sense in RISC-V
  /*if((tmp = strstr(soc_name, "???")) == NULL)
    return false;*/

  //soc->soc_vendor = ???

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

struct system_on_chip* parse_soc_from_string(struct system_on_chip* soc) {
  char* raw_name = soc->raw_name;

  if(match_starfive(raw_name, soc))
    return soc;

  if(match_allwinner(raw_name, soc))
    return soc;

  match_sifive(raw_name, soc);
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

struct system_on_chip* get_soc(void) {
  struct system_on_chip* soc = emalloc(sizeof(struct system_on_chip));
  soc->raw_name = NULL;
  soc->soc_vendor = SOC_VENDOR_UNKNOWN;
  soc->soc_model = SOC_MODEL_UNKNOWN;
  soc->process = UNKNOWN;

  soc = guess_soc_from_devtree(soc);
  if(soc->soc_vendor == SOC_VENDOR_UNKNOWN) {
    if(soc->raw_name != NULL) {
      printWarn("SoC detection failed using device tree: Found '%s' string", soc->raw_name);
    }
    else {
      printWarn("SoC detection failed using device tree");
    }
  }

  if(soc->soc_model == SOC_MODEL_UNKNOWN) {
    // raw_name might not be NULL, but if we were unable to find
    // the exact SoC, just print "Unkwnown"
    soc->raw_name = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }

  return soc;
}
