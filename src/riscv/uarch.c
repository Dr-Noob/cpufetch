#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uarch.h"
#include "../common/global.h"

typedef uint32_t MICROARCH;

struct uarch {
  MICROARCH uarch;
  char* uarch_str;
};

enum {
  UARCH_UNKNOWN,
  // SIFIVE
  UARCH_U74MC
};

#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, cpu, cpuinfo_str, uarch_str, str, uarch, vendor) \
   else if (strcmp(cpuinfo_str, uarch_str) == 0) fill_uarch(arch, cpu, str, uarch, vendor);
#define UARCH_END else { printBug("Unknown microarchitecture detected: uarch='%s'", cpuinfo_str); fill_uarch(arch, cpu, "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN); }

void fill_uarch(struct uarch* arch, struct cpuInfo* cpu, char* str, MICROARCH u, VENDOR vendor) {
  arch->uarch = u;
  cpu->cpu_vendor = vendor;
  arch->uarch_str = emalloc(sizeof(char) * (strlen(str)+1));
  strcpy(arch->uarch_str, str);
}

struct uarch* get_uarch_from_cpuinfo_str(char* cpuinfo_str, struct cpuInfo* cpu) {
  struct uarch* arch = emalloc(sizeof(struct uarch));
  if(cpuinfo_str == NULL) {
    printWarn("get_uarch_from_cpuinfo: Unable to detect microarchitecture, cpuinfo_str is NULL");
    fill_uarch(arch, cpu, "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN);
    return arch;
  }

  UARCH_START
  CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,u74-mc", "U74-MC", UARCH_U74MC, CPU_VENDOR_SIFIVE)
  UARCH_END

  return arch;
}

char* get_str_uarch(struct cpuInfo* cpu) {
  return cpu->arch->uarch_str;
}

void free_uarch_struct(struct uarch* arch) {
  free(arch->uarch_str);
  free(arch);
}
