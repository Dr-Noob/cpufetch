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
  UARCH_CELLBE,
  UARCH_POWER5,
  UARCH_POWER6,
  UARCH_POWER7,
  UARCH_POWER8,
  UARCH_POWER9,
  UARCH_POWER10,  
};

struct uarch {
  MICROARCH uarch;
  char* uarch_str;
  int32_t process; // measured in nanometers
};

#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, cpu_pvr, pvr_mask, pvr_value, str, uarch, process) \
   else if ((cpu_pvr & pvr_mask) == pvr_value) fill_uarch(arch, str, uarch, process);
#define UARCH_END else { printBug("Unknown microarchitecture detected: 0x%.8X", pvr); fill_uarch(arch, "Unknown", UARCH_UNKNOWN, 0); }

void fill_uarch(struct uarch* arch, char* str, MICROARCH u, uint32_t process) {
  arch->uarch_str = malloc(sizeof(char) * (strlen(str)+1));
  strcpy(arch->uarch_str, str);
  arch->uarch = u;
  arch->process= process;
}

/*
 * PVR masks/values from arch/powerpc/kernel/cputable.c (Linux kernel)
 * This list may be incorrect, incomplete or overly simplified, 
 * specially in the case of 32 bit entries
 */
struct uarch* get_uarch_from_pvr(uint32_t pvr) {
  struct uarch* arch = malloc(sizeof(struct uarch));

  UARCH_START
  // 64 bit
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00390000, "PPC970",       UARCH_PPC970,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003c0000, "PPC970",       UARCH_PPC970,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x00440100, "PPC970",       UARCH_PPC970,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00440000, "PPC970",       UARCH_PPC970,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00450000, "PPC970",       UARCH_PPC970,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003a0000, "POWER5",       UARCH_POWER5,  -1)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x003b0300, "POWER5",       UARCH_POWER5,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003b0000, "POWER5",       UARCH_POWER5,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000001, "POWER5",       UARCH_POWER5,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003e0000, "POWER6",       UARCH_POWER6,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000002, "POWER6",       UARCH_POWER6,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000003, "POWER7",       UARCH_POWER7,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000004, "POWER8",       UARCH_POWER8,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000005, "POWER9",       UARCH_POWER9,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x0f000006, "POWER10",      UARCH_POWER10, -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x003f0000, "POWER7",       UARCH_POWER7,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004A0000, "POWER7",       UARCH_POWER7,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004b0000, "POWER8",       UARCH_POWER8,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004c0000, "POWER8",       UARCH_POWER8,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004d0000, "POWER8",       UARCH_POWER8,  -1)
  CHECK_UARCH(arch, pvr, 0xffffefff, 0x004e0200, "POWER9",       UARCH_POWER9,  -1)
  CHECK_UARCH(arch, pvr, 0xffffefff, 0x004e0201, "POWER9",       UARCH_POWER9,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x004e0000, "POWER9",       UARCH_POWER9,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00800000, "POWER10",      UARCH_POWER10, -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00700000, "Cell BE",      UARCH_CELLBE,  -1)
  // 32 bit
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00040000, "PowerPC C604", UARCH_PPC604,  -1)
  CHECK_UARCH(arch, pvr, 0xfffff000, 0x00090000, "PowerPC C604", UARCH_PPC604,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00090000, "PowerPC C604", UARCH_PPC604,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x000a0000, "PowerPC C604", UARCH_PPC604,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x00084202, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x00080100, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x00082200, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x00082210, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x00083214, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xfffff0e0, 0x00087000, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xfffff000, 0x00083000, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x70000100, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x70000200, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x70000000, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x70020000, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00080000, "PowerPC G3",   UARCH_PPCG3,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x000c1101, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x000c0000, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x800c0000, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80000200, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80000201, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80000000, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x80010100, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80010200, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80010000, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80020100, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x80020101, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80020000, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80030000, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x80040000, "PowerPC G4",   UARCH_PPCG4,   -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00030000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00060000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00070000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00810000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00820000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00830000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00840000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00850000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0x7fff0000, 0x00860000, "PowerPC 603",  UARCH_PPC603,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41810000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41610000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x40B10000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41410000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x50910000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x51510000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41F10000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x51210000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910007, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x1291000d, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x1291000f, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910003, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910005, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910001, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910009, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x1291000b, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff000f, 0x12910002, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x41510000, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x7ff11432, "PowerPC 405",  UARCH_PPC405,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000850, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000858, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x400008d3, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000ff7, 0x400008d4, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x400008db, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000ffb, 0x200008D0, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000ffb, 0x200008D8, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000440, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x40000481, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000850, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000851, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000892, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xf0000fff, 0x50000894, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xfff00fff, 0x53200891, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xfff00fff, 0x53400890, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xfff00fff, 0x53400891, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0006, 0x13020002, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0007, 0x13020004, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0006, 0x13020000, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0007, 0x13020005, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xffffff00, 0x13541800, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xfffffff0, 0x12C41C80, "PowerPC 440",  UARCH_PPC440,  -1)
  CHECK_UARCH(arch, pvr, 0xffffffff, 0x11a52080, "PowerPC 470",  UARCH_PPC470,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x7ff50000, "PowerPC 470",  UARCH_PPC470,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x00050000, "PowerPC 470",  UARCH_PPC470,  -1)
  CHECK_UARCH(arch, pvr, 0xffff0000, 0x11a50000, "PowerPC 470",  UARCH_PPC470,  -1)
  UARCH_END

  return arch;
}

char* get_str_uarch(struct cpuInfo* cpu) {
  return cpu->arch->uarch_str;    
}

char* get_str_process(struct cpuInfo* cpu) {
  char* str = malloc(sizeof(char) * (strlen(STRING_UNKNOWN)+1));
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
