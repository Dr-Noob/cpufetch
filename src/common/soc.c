#include "soc.h"
#ifdef ARCH_ARM
  #include "../arm/socs.h"
#elif ARCH_RISCV
  #include "../riscv/socs.h"
#endif
#include "udev.h"
#include "../common/global.h"

#include <string.h>

static char* soc_trademark_string[] = {
  // ARM
  [SOC_VENDOR_SNAPDRAGON] = "Snapdragon ",
  [SOC_VENDOR_MEDIATEK]   = "MediaTek ",
  [SOC_VENDOR_EXYNOS]     = "Exynos ",
  [SOC_VENDOR_KIRIN]      = "Kirin ",
  [SOC_VENDOR_KUNPENG]    = "Kunpeng ",
  [SOC_VENDOR_BROADCOM]   = "Broadcom ",
  [SOC_VENDOR_APPLE]      = "Apple ",
  [SOC_VENDOR_ROCKCHIP]   = "Rockchip ",
  [SOC_VENDOR_GOOGLE]     = "Google ",
  [SOC_VENDOR_NVIDIA]     = "NVIDIA ",
  [SOC_VENDOR_AMPERE]     = "Ampere ",
  [SOC_VENDOR_NXP]        = "NXP ",
  [SOC_VENDOR_AMLOGIC]    = "Amlogic ",
  [SOC_VENDOR_MARVELL]    = "Marvell",
  // RISC-V
  [SOC_VENDOR_SIFIVE]     = "SiFive ",
  [SOC_VENDOR_STARFIVE]   = "StarFive ",
  [SOC_VENDOR_SIPEED]     = "Sipeed ",
  // ARM & RISC-V
  [SOC_VENDOR_ALLWINNER]  = "Allwinner "
};

VENDOR get_soc_vendor(struct system_on_chip* soc) {
  return soc->vendor;
}

char* get_str_process(struct system_on_chip* soc) {
  char* str;

  if(soc->process == UNKNOWN) {
    str = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(str, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  else {
    int max_process_len = 5 + 1;
    str = ecalloc(max_process_len, sizeof(char));
    snprintf(str, max_process_len, "%dnm", soc->process);
  }
  return str;
}

char* get_soc_name(struct system_on_chip* soc) {
  if(soc->model == SOC_MODEL_UNKNOWN)
    return soc->raw_name;
  return soc->name;
}

void fill_soc(struct system_on_chip* soc, char* soc_name, SOC soc_model, int32_t process) {
  soc->model = soc_model;
  soc->vendor = get_soc_vendor_from_soc(soc_model);
  soc->process = process;
  if(soc->vendor == SOC_VENDOR_UNKNOWN) {
    printBug("fill_soc: soc->vendor == SOC_VENDOR_UNKOWN");
    // If we fall here there is a bug in socs.h
    // Reset everything to avoid segfault
    soc->vendor = SOC_VENDOR_UNKNOWN;
    soc->model = SOC_MODEL_UNKNOWN;
    soc->process = UNKNOWN;
    soc->raw_name = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  else {
    int len = strlen(soc_name) + strlen(soc_trademark_string[soc->vendor]) + 1;
    soc->name = emalloc(sizeof(char) * len);
    sprintf(soc->name, "%s%s", soc_trademark_string[soc->vendor], soc_name);
  }
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
