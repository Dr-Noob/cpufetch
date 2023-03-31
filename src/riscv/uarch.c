#include <stdint.h>

struct uarch {
  //MICROARCH uarch;
  char* uarch_str;
  int32_t process; // measured in nanometers
};

void free_uarch_struct(struct uarch* arch) {

}
