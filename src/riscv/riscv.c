#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../common/global.h"
#include "../common/udev.h"
#include "udev.h"
#include "uarch.h"
#include "soc.h"

#define SET_ISA_EXT_MAP(name, bit)             \
  if(strncmp(multi_letter_extension, name,     \
             multi_letter_extension_len) == 0) \
    ext->mask |= 1UL << bit;                   \

struct frequency* get_frequency_info(uint32_t core) {
  struct frequency* freq = emalloc(sizeof(struct frequency));

  freq->base = UNKNOWN_DATA;
  freq->max = get_max_freq_from_file(core);

  return freq;
}

int64_t get_peak_performance(struct cpuInfo* cpu) {
  //First check we have consistent data
  if(get_freq(cpu->freq) == UNKNOWN_DATA) {
    return -1;
  }

  int64_t flops = cpu->topo->total_cores * (get_freq(cpu->freq) * 1000000);
  return flops;
}

// Returns the length of the multi-letter
// extension, or -1 if an error occurs
int parse_multi_letter_extension(struct extensions* ext, char* e) {
  if(*e != '_') return -1;
  char* multi_letter_extension_end = strstr(e+1, "_");
  if(multi_letter_extension_end == NULL) {
    // This is the last extension, find the end
    // of the string
    multi_letter_extension_end = e + strlen(e);
  }

  int multi_letter_extension_len = multi_letter_extension_end-(e+1);

  char* multi_letter_extension = emalloc(multi_letter_extension_len);
  strncpy(multi_letter_extension, e+1, multi_letter_extension_len);
  // TODO: Add more extensions
  // https://en.wikipedia.org/wiki/RISC-V
  SET_ISA_EXT_MAP("smaia",       RISCV_ISA_EXT_SMAIA);
  SET_ISA_EXT_MAP("ssaia",       RISCV_ISA_EXT_SSAIA);
  SET_ISA_EXT_MAP("sscofpmf",    RISCV_ISA_EXT_SSCOFPMF);
  SET_ISA_EXT_MAP("sstc",        RISCV_ISA_EXT_SSTC);
  SET_ISA_EXT_MAP("svinval",     RISCV_ISA_EXT_SVINVAL);
  SET_ISA_EXT_MAP("svnapot",     RISCV_ISA_EXT_SVNAPOT);
  SET_ISA_EXT_MAP("svpbmt",      RISCV_ISA_EXT_SVPBMT);
  SET_ISA_EXT_MAP("zba",         RISCV_ISA_EXT_ZBA);
  SET_ISA_EXT_MAP("zbb",         RISCV_ISA_EXT_ZBB);
  SET_ISA_EXT_MAP("zbs",         RISCV_ISA_EXT_ZBS);
  SET_ISA_EXT_MAP("zicbom",      RISCV_ISA_EXT_ZICBOM);
  SET_ISA_EXT_MAP("zicboz",      RISCV_ISA_EXT_ZICBOZ);
  SET_ISA_EXT_MAP("zihintpause", RISCV_ISA_EXT_ZIHINTPAUSE);
  else {
    printBug("parse_multi_letter_extension: Unknown multi-letter extension: %s", multi_letter_extension);
    return -1;
  }

  return multi_letter_extension_len;
}

struct extensions* get_extensions_from_str(char* str) {
  struct extensions* ext = emalloc(sizeof(struct extensions));
  ext->mask = 0;
  ext->str = NULL;

  if(str == NULL) {
    return ext;
  }

  int len = sizeof(char) * (strlen(str)+1);
  ext->str = emalloc(sizeof(char) * len);
  memset(ext->str, 0, len);
  strncpy(ext->str, str, sizeof(char) * len);

  // Code inspired in Linux kernel (riscv_fill_hwcap):
  // https://elixir.bootlin.com/linux/v6.2.10/source/arch/riscv/kernel/cpufeature.c
  char* isa = str;
  if (!strncmp(isa, "rv32", 4))
    isa += 4;
  else if (!strncmp(isa, "rv64", 4))
    isa += 4;
  else {
    printBug("get_extensions_from_str: ISA string must start with rv64 or rv32");
    return ext;
  }

  for(char* e = isa; *e != '\0'; e++) {
    if(*e == '_') {
      // Multi-letter extension
      int multi_letter_extension_len = parse_multi_letter_extension(ext, e);
      if(multi_letter_extension_len == -1) {
        return ext;
      }
      e += multi_letter_extension_len;
    }
    else {
      int n = *e - 'a';
      ext->mask |= 1UL << n;
    }
  }

  return ext;
}

struct cpuInfo* get_cpu_info(void) {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  //init_cpu_info(cpu);
  struct topology* topo = emalloc(sizeof(struct topology));
  topo->total_cores = get_ncores_from_cpuinfo();
  topo->cach = NULL;
  cpu->topo = topo;

  char* cpuinfo_str = get_uarch_from_cpuinfo();
  char* ext_str = get_extensions_from_cpuinfo();
  cpu->hv = emalloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  cpu->ext = get_extensions_from_str(ext_str);
  if(cpu->ext->str != NULL && cpu->ext->mask == 0) return NULL;
  cpu->arch = get_uarch_from_cpuinfo_str(cpuinfo_str, cpu);
  cpu->soc = get_soc();
  cpu->freq = get_frequency_info(0);
  cpu->peak_performance = get_peak_performance(cpu);

  return cpu;
}

//TODO: Might be worth refactoring with other archs
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo) {
  uint32_t size = 3+7+1;
  char* string = emalloc(sizeof(char)*size);
  snprintf(string, size, "%d cores", topo->total_cores);

  return string;
}

char* get_str_extensions(struct cpuInfo* cpu) {
  if(cpu->ext != NULL) {
    return cpu->ext->str;
  }
  return NULL;
}

void print_debug(struct cpuInfo* cpu) {
  printf("- soc: ");
  if(cpu->soc->raw_name == NULL) {
    printf("NULL\n");
  }
  else {
    printf("'%s'\n", cpu->soc->raw_name);
  }

  printf("- uarch: ");
  char* arch_cpuinfo_str = get_arch_cpuinfo_str(cpu);
  if(arch_cpuinfo_str == NULL) {
    printf("NULL\n");
  }
  else {
    printf("'%s'\n", arch_cpuinfo_str);
  }
}
