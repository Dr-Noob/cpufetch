#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../common/global.h"
#include "../common/cpu.h"

struct topology* get_topology_from_sysctl() {
  struct topology* t = malloc(sizeof(struct topology));
  size_t dummy;

  if(sysctlbyname("hw.physicalcpu_max", &t->total_cores, &dummy, NULL, 0) != 0) {
    printWarn("sysctlbyname(\"hw.physicalcpu_max\") failed: %s\n", strerror(errno));
    t->total_cores = 1;
  }
  else if(t->total_cores <= 0) {
    printWarn("sysctlbyname(\"hw.physicalcpu_max\") returned invalid value: %d\n", t->total_cores);
    t->total_cores = 1;
  }

  return t;
}

