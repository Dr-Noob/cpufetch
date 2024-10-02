#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uarch.h"
#include "udev.h"
#include "../common/global.h"

typedef uint32_t MICROARCH;

struct uarch {
  MICROARCH uarch;
  char* uarch_str;
  char* cpuinfo_str;
  struct riscv_cpuinfo* ci;
};

enum {
  UARCH_UNKNOWN,
  // SIFIVE
  UARCH_U54,
  UARCH_U74,
  // THEAD
  UARCH_C906,
  UARCH_C910,
  // SPACEMIT
  UARCH_X60
};

#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, cpu, cpuinfo_str, uarch_str, str, uarch, vendor) \
   else if (strcmp(cpuinfo_str, uarch_str) == 0) fill_uarch(arch, cpu, str, uarch, vendor);
#define UARCH_END else { printWarn("Unknown microarchitecture detected: uarch='%s'", cpuinfo_str); fill_uarch(arch, cpu, "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN); }

#define ARCHID_START if (false) {}
#define CHECK_ARCHID(arch, marchid_val, str, uarch, vendor) \
  else if (arch->ci->marchid == (unsigned long) marchid_val) fill_uarch(arch, cpu, str, uarch, vendor);
#define ARCHID_END else { printWarn("Unknown microarchitecture detected: marchid=0x%.8X", arch->ci->marchid); fill_uarch(arch, cpu, "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN); }

void fill_uarch(struct uarch* arch, struct cpuInfo* cpu, char* str, MICROARCH u, VENDOR vendor) {
  arch->uarch = u;
  cpu->cpu_vendor = vendor;
  arch->uarch_str = emalloc(sizeof(char) * (strlen(str)+1));
  strcpy(arch->uarch_str, str);
}

// https://elixir.bootlin.com/linux/latest/source/Documentation/devicetree/bindings/riscv/cpus.yaml
// SiFive: https://www.sifive.com/risc-v-core-ip
// T-Head: https://www.t-head.cn/product/c906
struct uarch* get_uarch_from_cpuinfo_str(char* cpuinfo_str, struct cpuInfo* cpu, struct uarch* arch) {
  arch->cpuinfo_str = cpuinfo_str;

  // U74/U74-MC:
  // SiFive says that U74-MC is "Multicore: four U74 cores and one S76 core" while
  // U74 is "High performance Linux-capable processor". It's like U74-MC is somehow a small SoC containing
  // the U74 and the S76? Then U74-MC is not a microarchitecture per se...
  UARCH_START
  CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,bullet0", "U74",         UARCH_U74, CPU_VENDOR_SIFIVE) // bullet0 is present in U740, which has U74
  // CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,e5",      "XXXXXX",      UARCH_U74, CPU_VENDOR_SIFIVE)
  // CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,e7",      "XXXXXX",      UARCH_U74, CPU_VENDOR_SIFIVE)
  // CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,e71",     "XXXXXX",      UARCH_U74, CPU_VENDOR_SIFIVE)
  // CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,rocket0", "XXXXXX",      UARCH_U74, CPU_VENDOR_SIFIVE)
  // CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,u5",      "XXXXXX",      UARCH_U74, CPU_VENDOR_SIFIVE)
  CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,u54",     "U54",         UARCH_U54,  CPU_VENDOR_SIFIVE)
  // CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,u7",      "XXXXXX",      UARCH_U74, CPU_VENDOR_SIFIVE)
  CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,u74",     "U74",         UARCH_U74,  CPU_VENDOR_SIFIVE)
  CHECK_UARCH(arch, cpu, cpuinfo_str, "sifive,u74-mc",  "U74",         UARCH_U74,  CPU_VENDOR_SIFIVE)
  CHECK_UARCH(arch, cpu, cpuinfo_str, "thead,c906",     "T-Head C906", UARCH_C906, CPU_VENDOR_THEAD)
  CHECK_UARCH(arch, cpu, cpuinfo_str, "thead,c910",     "T-Head C910", UARCH_C910, CPU_VENDOR_THEAD)
  UARCH_END

  return arch;
}

// TODO: Doc
struct uarch* get_uarch_from_riscv_cpuinfo(struct cpuInfo* cpu, struct uarch* arch) {

  ARCHID_START
  CHECK_ARCHID(arch, 0x8000000058000001, "X60", UARCH_X60, CPU_VENDOR_SPACEMIT)
  ARCHID_END

  return arch;
}

struct uarch* get_uarch(struct cpuInfo* cpu) {
  char* cpuinfo_str = get_uarch_from_cpuinfo();
  struct uarch* arch = emalloc(sizeof(struct uarch));
  arch->uarch = UARCH_UNKNOWN;
  arch->ci = NULL;

  if (cpuinfo_str == NULL) {
    printWarn("get_uarch_from_cpuinfo: Unable to detect microarchitecture using uarch: cpuinfo_str is NULL");
    arch->ci = get_riscv_cpuinfo();

    if (arch->ci == NULL)
      printWarn("get_riscv_cpuinfo: Unable to get cpuinfo from udev");
    else
      arch = get_uarch_from_riscv_cpuinfo(cpu, arch);
  }
  else {
    arch = get_uarch_from_cpuinfo_str(cpuinfo_str, cpu, arch);
  }

  if (arch->uarch == UARCH_UNKNOWN)
    fill_uarch(arch, cpu, "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN);

  return arch;
}

char* get_str_uarch(struct cpuInfo* cpu) {
  return cpu->arch->uarch_str;
}

char* get_arch_cpuinfo_str(struct cpuInfo* cpu) {
  return cpu->arch->cpuinfo_str;
}

void free_uarch_struct(struct uarch* arch) {
  free(arch->uarch_str);
  free(arch);
}
