#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../common/global.h"
#include "../common/udev.h"
#include "udev.h"
#include "uarch.h"
#include "soc.h"

#define SET_ISA_EXT_MAP(name, bit)          \
  if(strncmp(multi_letter_extension, name,  \
        multi_letter_extension_len) == 0) { \
    ext->mask |= 1UL << bit;                \
    maskset = true;                         \
  }                                         \

struct frequency* get_frequency_info(uint32_t core) {
  struct frequency* freq = emalloc(sizeof(struct frequency));

  freq->measured = false;
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
  bool maskset = false;
  char* multi_letter_extension = emalloc(multi_letter_extension_len);
  strncpy(multi_letter_extension, e+1, multi_letter_extension_len);
  // This should be up-to-date with
  // https://elixir.bootlin.com/linux/latest/source/arch/riscv/kernel/cpufeature.c
  // which should represent the list of extensions available in real chips
  SET_ISA_EXT_MAP("sscofpmf",    RISCV_ISA_EXT_SSCOFPMF)
  SET_ISA_EXT_MAP("sstc",        RISCV_ISA_EXT_SSTC)
  SET_ISA_EXT_MAP("svinval",     RISCV_ISA_EXT_SVINVAL)
  SET_ISA_EXT_MAP("svpbmt",      RISCV_ISA_EXT_SVPBMT)
  SET_ISA_EXT_MAP("zbb",         RISCV_ISA_EXT_ZBB)
  SET_ISA_EXT_MAP("zicbom",      RISCV_ISA_EXT_ZICBOM)
  SET_ISA_EXT_MAP("zihintpause", RISCV_ISA_EXT_ZIHINTPAUSE)
  SET_ISA_EXT_MAP("svnapot",     RISCV_ISA_EXT_SVNAPOT)
  SET_ISA_EXT_MAP("zicboz",      RISCV_ISA_EXT_ZICBOZ)
  SET_ISA_EXT_MAP("smaia",       RISCV_ISA_EXT_SMAIA)
  SET_ISA_EXT_MAP("ssaia",       RISCV_ISA_EXT_SSAIA)
  SET_ISA_EXT_MAP("zba",         RISCV_ISA_EXT_ZBA)
  SET_ISA_EXT_MAP("zbs",         RISCV_ISA_EXT_ZBS)
  SET_ISA_EXT_MAP("zicntr",      RISCV_ISA_EXT_ZICNTR)
  SET_ISA_EXT_MAP("zicsr",       RISCV_ISA_EXT_ZICSR)
  SET_ISA_EXT_MAP("zifencei",    RISCV_ISA_EXT_ZIFENCEI)
  SET_ISA_EXT_MAP("zihpm",       RISCV_ISA_EXT_ZIHPM)
  SET_ISA_EXT_MAP("smstateen",   RISCV_ISA_EXT_SMSTATEEN)
  SET_ISA_EXT_MAP("zicond",      RISCV_ISA_EXT_ZICOND)
  SET_ISA_EXT_MAP("zbc",         RISCV_ISA_EXT_ZBC)
  SET_ISA_EXT_MAP("zbkb",        RISCV_ISA_EXT_ZBKB)
  SET_ISA_EXT_MAP("zbkc",        RISCV_ISA_EXT_ZBKC)
  SET_ISA_EXT_MAP("zbkx",        RISCV_ISA_EXT_ZBKX)
  SET_ISA_EXT_MAP("zknd",        RISCV_ISA_EXT_ZKND)
  SET_ISA_EXT_MAP("zkne",        RISCV_ISA_EXT_ZKNE)
  SET_ISA_EXT_MAP("zknh",        RISCV_ISA_EXT_ZKNH)
  SET_ISA_EXT_MAP("zkr",         RISCV_ISA_EXT_ZKR)
  SET_ISA_EXT_MAP("zksed",       RISCV_ISA_EXT_ZKSED)
  SET_ISA_EXT_MAP("zksh",        RISCV_ISA_EXT_ZKSH)
  SET_ISA_EXT_MAP("zkt",         RISCV_ISA_EXT_ZKT)
  SET_ISA_EXT_MAP("zvbb",        RISCV_ISA_EXT_ZVBB)
  SET_ISA_EXT_MAP("zvbc",        RISCV_ISA_EXT_ZVBC)
  SET_ISA_EXT_MAP("zvkb",        RISCV_ISA_EXT_ZVKB)
  SET_ISA_EXT_MAP("zvkg",        RISCV_ISA_EXT_ZVKG)
  SET_ISA_EXT_MAP("zvkned",      RISCV_ISA_EXT_ZVKNED)
  SET_ISA_EXT_MAP("zvknha",      RISCV_ISA_EXT_ZVKNHA)
  SET_ISA_EXT_MAP("zvknhb",      RISCV_ISA_EXT_ZVKNHB)
  SET_ISA_EXT_MAP("zvksed",      RISCV_ISA_EXT_ZVKSED)
  SET_ISA_EXT_MAP("zvksh",       RISCV_ISA_EXT_ZVKSH)
  SET_ISA_EXT_MAP("zvkt",        RISCV_ISA_EXT_ZVKT)
  SET_ISA_EXT_MAP("zfh",         RISCV_ISA_EXT_ZFH)
  SET_ISA_EXT_MAP("zfhmin",      RISCV_ISA_EXT_ZFHMIN)
  SET_ISA_EXT_MAP("zihintntl",   RISCV_ISA_EXT_ZIHINTNTL)
  SET_ISA_EXT_MAP("zvfh",        RISCV_ISA_EXT_ZVFH)
  SET_ISA_EXT_MAP("zvfhmin",     RISCV_ISA_EXT_ZVFHMIN)
  SET_ISA_EXT_MAP("zfa",         RISCV_ISA_EXT_ZFA)
  SET_ISA_EXT_MAP("ztso",        RISCV_ISA_EXT_ZTSO)
  SET_ISA_EXT_MAP("zacas",       RISCV_ISA_EXT_ZACAS)
  SET_ISA_EXT_MAP("zve32x",      RISCV_ISA_EXT_ZVE32X)
  SET_ISA_EXT_MAP("zve32f",      RISCV_ISA_EXT_ZVE32F)
  SET_ISA_EXT_MAP("zve64x",      RISCV_ISA_EXT_ZVE64X)
  SET_ISA_EXT_MAP("zve64f",      RISCV_ISA_EXT_ZVE64F)
  SET_ISA_EXT_MAP("zve64d",      RISCV_ISA_EXT_ZVE64D)
  SET_ISA_EXT_MAP("zimop",       RISCV_ISA_EXT_ZIMOP)
  SET_ISA_EXT_MAP("zca",         RISCV_ISA_EXT_ZCA)
  SET_ISA_EXT_MAP("zcb",         RISCV_ISA_EXT_ZCB)
  SET_ISA_EXT_MAP("zcd",         RISCV_ISA_EXT_ZCD)
  SET_ISA_EXT_MAP("zcf",         RISCV_ISA_EXT_ZCF)
  SET_ISA_EXT_MAP("zcmop",       RISCV_ISA_EXT_ZCMOP)
  SET_ISA_EXT_MAP("zawrs",       RISCV_ISA_EXT_ZAWRS)
  SET_ISA_EXT_MAP("svvptc",      RISCV_ISA_EXT_SVVPTC)
  SET_ISA_EXT_MAP("smmpm",       RISCV_ISA_EXT_SMMPM)
  SET_ISA_EXT_MAP("smnpm",       RISCV_ISA_EXT_SMNPM)
  SET_ISA_EXT_MAP("ssnpm",       RISCV_ISA_EXT_SSNPM)
  SET_ISA_EXT_MAP("zabha",       RISCV_ISA_EXT_ZABHA)
  SET_ISA_EXT_MAP("ziccrse",     RISCV_ISA_EXT_ZICCRSE)
  SET_ISA_EXT_MAP("svade",       RISCV_ISA_EXT_SVADE)
  SET_ISA_EXT_MAP("svadu",       RISCV_ISA_EXT_SVADU)
  SET_ISA_EXT_MAP("zfbfmin",     RISCV_ISA_EXT_ZFBFMIN)
  SET_ISA_EXT_MAP("zvfbfmin",    RISCV_ISA_EXT_ZVFBFMIN)
  SET_ISA_EXT_MAP("zvfbfwma",    RISCV_ISA_EXT_ZVFBFWMA)
  SET_ISA_EXT_MAP("zaamo",       RISCV_ISA_EXT_ZAAMO)
  SET_ISA_EXT_MAP("zalrsc",      RISCV_ISA_EXT_ZALRSC)
  SET_ISA_EXT_MAP("zicbop",      RISCV_ISA_EXT_ZICBOP)
  SET_ISA_EXT_MAP("ime",         RISCV_ISA_EXT_IME)

  if(!maskset) {
    printBug("parse_multi_letter_extension: Unknown multi-letter extension: %s", multi_letter_extension);
    return -1;
  }

  return multi_letter_extension_len;
}

bool valid_extension(char ext) {
  bool found = false;
  uint64_t idx = 0;

  while(idx < sizeof(extension_list)/sizeof(extension_list[0]) && !found) {
    found = (extension_list[idx].id == (ext - 'a'));
    if(!found) idx++;
  }

  return found;
}

struct extensions* get_extensions_from_str(char* str) {
  struct extensions* ext = emalloc(sizeof(struct extensions));
  ext->mask = 0;
  ext->str = NULL;

  if(str == NULL) {
    return ext;
  }

  int len = strlen(str)+1;
  ext->str = emalloc(len * sizeof(char));
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
      // Single-letter extensions 's' and 'u' are invalid
      // according to Linux kernel (arch/riscv/kernel/cpufeature.c:
      // riscv_fill_hwcap). Optionally, we could opt for using
      // hwcap instead of cpuinfo to avoid this
      if (*e == 's' || *e == 'u') {
        continue;
      }
      // Make sure that the extension is valid before
      // adding it to the mask
      if(valid_extension(*e)) {
        int n = *e - 'a';
        ext->mask |= 1UL << n;
      }
      else {
        printBug("get_extensions_from_str: Invalid extension: '%c'", *e);
      }
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

  char* ext_str = get_extensions_from_cpuinfo();
  cpu->hv = emalloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  cpu->ext = get_extensions_from_str(ext_str);
  if(cpu->ext->str != NULL && cpu->ext->mask == 0) return NULL;
  cpu->arch = get_uarch(cpu);
  cpu->soc = get_soc(cpu);
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
