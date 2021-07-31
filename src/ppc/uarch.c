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
  UARCH_POWER9
};

struct uarch {
  MICROARCH uarch;
  char* uarch_str;
  int32_t process; // measured in nanometers
};

#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, auxv_plat_ret, auxv_plat, str, uarch, process) \
   else if (strcmp(auxv_plat_ret, auxv_plat) == 0) fill_uarch(arch, str, uarch, process);
#define UARCH_END else { printBug("Unknown microarchitecture detected: '%s'", platform); fill_uarch(arch, "Unknown", UARCH_UNKNOWN, 0); }

void fill_uarch(struct uarch* arch, char* str, MICROARCH u, uint32_t process) {
  arch->uarch_str = malloc(sizeof(char) * (strlen(str)+1));
  strcpy(arch->uarch_str, str);
  arch->uarch = u;
  arch->process= process;
}

struct uarch* get_uarch_from_auxval() {
  struct uarch* arch = malloc(sizeof(struct uarch));

  unsigned long ret = getauxval(AT_PLATFORM);
  if(ret == 0 && errno == ENOENT) {
    printErr("Entry AT_PLATFORM not found in getauxval");
    perror("getauxval");
  }

  char* platform = (char *) ret;

  UARCH_START
  CHECK_UARCH(arch, platform, "power9", "POWER 9", UARCH_POWER9, 14)
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
