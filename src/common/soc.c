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
  [SOC_VENDOR_BROADCOM]   = "Broadcom BCM",
  [SOC_VENDOR_APPLE]      = "Apple ",
  [SOC_VENDOR_ROCKCHIP]   = "Rockchip ",
  [SOC_VENDOR_GOOGLE]     = "Google ",
  [SOC_VENDOR_NVIDIA]     = "NVIDIA ",
  // RISC-V
  [SOC_VENDOR_SIFIVE]     = "SiFive ",
  [SOC_VENDOR_STARFIVE]   = "StarFive ",
  [SOC_VENDOR_SIPEED]     = "Sipeed ",
  // ARM & RISC-V
  [SOC_VENDOR_ALLWINNER]  = "Allwinner "
};

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

void fill_soc(struct system_on_chip* soc, char* soc_name, SOC soc_model, int32_t process) {
  soc->soc_model = soc_model;
  soc->soc_vendor = get_soc_vendor_from_soc(soc_model);
  soc->process = process;
  if(soc->soc_vendor == SOC_VENDOR_UNKNOWN) {
    printBug("fill_soc: soc->soc_vendor == SOC_VENDOR_UNKOWN");
    // If we fall here there is a bug in socs.h
    // Reset everything to avoid segfault
    soc->soc_vendor = SOC_VENDOR_UNKNOWN;
    soc->soc_model = SOC_MODEL_UNKNOWN;
    soc->process = UNKNOWN;
    soc->raw_name = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
    snprintf(soc->raw_name, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  else {
    soc->process = process;
    int len = strlen(soc_name) + strlen(soc_trademark_string[soc->soc_vendor]) + 1;
    soc->soc_name = emalloc(sizeof(char) * len);
    memset(soc->soc_name, 0, sizeof(char) * len);
    sprintf(soc->soc_name, "%s%s", soc_trademark_string[soc->soc_vendor], soc_name);
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
