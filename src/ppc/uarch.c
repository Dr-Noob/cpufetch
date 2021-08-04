#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/auxv.h>
#include <errno.h>

#include "uarch.h"
#include "../common/global.h"

typedef uint32_t MICROARCH;

#define STRING_UNKNOWN    "Unknown"

// Data not available
#define NA                   -1

// Unknown manufacturing process
#define UNK                  -1

enum {
  UARCH_UNKNOWN,
  UARCH_PPC604,
  UARCH_PPCG3,
  UARCH_PPCG4,
  UARCH_PPC405,
  UARCH_PPC603,
  UARCH_PPC440,
  UARCH_PPC470,
  UARCH_PPC970,
  UARCH_PPC970FX,
  UARCH_PPC970MP,
  UARCH_CELLBE,
  UARCH_POWER5,
  UARCH_POWER5PLUS,
  UARCH_POWER6,
  UARCH_POWER7,
  UARCH_POWER7PLUS,
  UARCH_POWER8,
  UARCH_POWER9,
  UARCH_POWER9_DD20,
  UARCH_POWER9_DD21,
  UARCH_POWER9_DD22,
  UARCH_POWER9_DD23,
  UARCH_POWER10,
};

struct uarch {
  MICROARCH uarch;
  char* uarch_str;
  int32_t process; // measured in nanometers
};

#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, cpu_pvr, pvr_mask, pvr_value, uarch) \
   else if ((cpu_pvr & pvr_mask) == pvr_value) fill_uarch(arch, uarch);
#define UARCH_END else { printBug("Unknown microarchitecture detected: 0x%.8X", pvr); fill_uarch(arch, UARCH_UNKNOWN); }

#define FILL_START if (false) {}
#define FILL_UARCH(u, uarch, uarch_str, uarch_process) \
   else if(u == uarch) { fill = true; str = uarch_str; process = uarch_process; }
#define FILL_END else { printBug("Found invalid microarchitecture: %d", u); }

void fill_uarch(struct uarch* arch, MICROARCH u) {
  arch->uarch = u;
  char* str = NULL;
  int32_t process = UNK;
  bool fill = false;

  FILL_START
  FILL_UARCH(arch->uarch, UARCH_UNKNOWN,     "Unknown",       UNK)
  FILL_UARCH(arch->uarch, UARCH_PPC604,      "PowerPC 604",   500)
  FILL_UARCH(arch->uarch, UARCH_PPCG3,       "PowerPC G3",    UNK) // varies
  FILL_UARCH(arch->uarch, UARCH_PPCG4,       "PowerPC G4",    UNK) // varies
  FILL_UARCH(arch->uarch, UARCH_PPC405,      "PowerPC 405",   UNK)
  FILL_UARCH(arch->uarch, UARCH_PPC603,      "PowerPC 603",   UNK) // varies
  FILL_UARCH(arch->uarch, UARCH_PPC440,      "PowerPC 440",   UNK)
  FILL_UARCH(arch->uarch, UARCH_PPC470,      "PowerPC 470",    45) // strange...
  FILL_UARCH(arch->uarch, UARCH_PPC970,      "PowerPC 970",   130)
  FILL_UARCH(arch->uarch, UARCH_PPC970FX,    "PowerPC 970FX",  90)
  FILL_UARCH(arch->uarch, UARCH_PPC970MP,    "PowerPC 970MP",  90)
  FILL_UARCH(arch->uarch, UARCH_CELLBE,      "Cell BE",       UNK) // varies depending on manufacturer
  FILL_UARCH(arch->uarch, UARCH_POWER5,      "POWER5",        130)
  FILL_UARCH(arch->uarch, UARCH_POWER5PLUS,  "POWER5+",        90)
  FILL_UARCH(arch->uarch, UARCH_POWER6,      "POWER6",         65)
  FILL_UARCH(arch->uarch, UARCH_POWER7,      "POWER7",         45)
  FILL_UARCH(arch->uarch, UARCH_POWER7PLUS,  "POWER7+",        32)
  FILL_UARCH(arch->uarch, UARCH_POWER8,      "POWER8",         22)
  FILL_UARCH(arch->uarch, UARCH_POWER9,      "POWER9",         14)
  FILL_UARCH(arch->uarch, UARCH_POWER9_DD20, "POWER9 (DD2.0)", 14)
  FILL_UARCH(arch->uarch, UARCH_POWER9_DD21, "POWER9 (DD2.1)", 14)
  FILL_UARCH(arch->uarch, UARCH_POWER9_DD22, "POWER9 (DD2.2)", 14)
  FILL_UARCH(arch->uarch, UARCH_POWER9_DD23, "POWER9 (DD2.3)", 14)
  FILL_UARCH(arch->uarch, UARCH_POWER10,     "POWER10",         7)
  FILL_END

  if(fill) {
    arch->uarch_str = emalloc(sizeof(char) * (strlen(str)+1));
    strcpy(arch->uarch_str, str);
    arch->process= process;
  }
}

/*
 * PVR masks/values from arch/powerpc/kernel/cputable.c (Linux kernel)
 * This list may be incorrect, incomplete or overly simplified,
 * specially in the case of 32 bit entries
 */
struct uarch* get_uarch_from_pvr(uint32_t pvr) {
  struct uarch* arch = emalloc(sizeof(struct uarch));

  UARCH_START
  // 64 bit
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00390000, UARCH_PPC970)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003c0000, UARCH_PPC970FX)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x00440100, UARCH_PPC970MP)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00440000, UARCH_PPC970MP)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003a0000, UARCH_POWER5)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x003b0300, UARCH_POWER5PLUS)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003b0000, UARCH_POWER5)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000001, UARCH_POWER5)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003e0000, UARCH_POWER6)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000002, UARCH_POWER6)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000003, UARCH_POWER7)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000004, UARCH_POWER8)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000005, UARCH_POWER9)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000006, UARCH_POWER10)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003f0000, UARCH_POWER7)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004A0000, UARCH_POWER7PLUS)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004b0000, UARCH_POWER8)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004c0000, UARCH_POWER8)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004d0000, UARCH_POWER8)
  CHECK_UARCH(arch, pvr, 0xffffefff, 0x004e0200, UARCH_POWER9_DD20)
  CHECK_UARCH(arch, pvr, 0xffffefff, 0x004e0201, UARCH_POWER9_DD21)
  CHECK_UARCH(arch, pvr, 0xffffefff, 0x004e0202, UARCH_POWER9_DD22)
  CHECK_UARCH(arch, pvr, 0xffffefff, 0x004e0203, UARCH_POWER9_DD23)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00800000, UARCH_POWER10)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00700000, UARCH_CELLBE)
  // 32 bit
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00040000, UARCH_PPC604)
  CHECK_UARCH(arch, pvr, 0xfffff000, 0x00090000, UARCH_PPC604)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00090000, UARCH_PPC604)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x000a0000, UARCH_PPC604)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x00084202, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x00080100, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x00082200, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x00082210, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x00083214, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xfffff0e0, 0x00087000, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xfffff000, 0x00083000, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x70000100, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x70000200, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x70000000, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x70020000, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00080000, UARCH_PPCG3)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x000c1101, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x000c0000, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x800c0000, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80000200, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80000201, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80000000, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x80010100, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80010200, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80010000, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80020100, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80020101, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80020000, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80030000, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80040000, UARCH_PPCG4)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00030000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00060000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00070000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00810000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00820000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00830000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00840000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00850000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00860000, UARCH_PPC603)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41810000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41610000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x40B10000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41410000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x50910000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x51510000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41F10000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x51210000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910007, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x1291000d, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x1291000f, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910003, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910005, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910001, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910009, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x1291000b, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910002, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41510000, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x7ff11432, UARCH_PPC405)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000850, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000858, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x400008d3, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000ff7, 0x400008d4, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x400008db, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000ffb, 0x200008D0, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000ffb, 0x200008D8, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000440, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000481, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000850, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000851, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000892, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000894, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xfff00fff, 0x53200891, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xfff00fff, 0x53400890, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xfff00fff, 0x53400891, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xffff0006, 0x13020002, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xffff0007, 0x13020004, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xffff0006, 0x13020000, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xffff0007, 0x13020005, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x13541800, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x12C41C80, UARCH_PPC440)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x11a52080, UARCH_PPC470)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x7ff50000, UARCH_PPC470)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00050000, UARCH_PPC470)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x11a50000, UARCH_PPC470)
  UARCH_END

  return arch;
}

bool has_altivec(struct uarch* arch) {
  switch(arch->uarch) {
    case UARCH_PPC970FX:
    case UARCH_PPC970MP:
    case UARCH_CELLBE:
    case UARCH_POWER6:
    case UARCH_POWER7:
    case UARCH_POWER7PLUS:
    case UARCH_POWER8:
    case UARCH_POWER9:
    case UARCH_POWER9_DD20:
    case UARCH_POWER9_DD21:
    case UARCH_POWER9_DD22:
    case UARCH_POWER9_DD23:
    case UARCH_POWER10:
      return true;
    default:
      return false;
  }
}

char* get_str_uarch(struct cpuInfo* cpu) {
  return cpu->arch->uarch_str;
}

char* get_str_process(struct cpuInfo* cpu) {
  char* str = emalloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
  int32_t process = cpu->arch->process;

  if(process == UNK) {
    snprintf(str, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
  }
  else if(process > 100) {
    sprintf(str, "%.2fum", (double)process/100);
  }
  else if(process > 0){
    sprintf(str, "%dnm", process);
  }
  else {
    snprintf(str, strlen(STRING_UNKNOWN)+1, STRING_UNKNOWN);
    printBug("Found invalid process: '%d'", process);
  }

  return str;
}

void free_uarch_struct(struct uarch* arch) {
  free(arch->uarch_str);
  free(arch);
}
