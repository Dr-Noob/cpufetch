#include <stdint.h>

#include "uarch.h"
#include "../common/global.h"

struct uarch {
  //MICROARCH uarch;
  char* uarch_str;
  int32_t process; // measured in nanometers
};

char* get_str_uarch(struct cpuInfo* cpu) {
  return "Unknown";
}

void free_uarch_struct(struct uarch* arch) {

}
