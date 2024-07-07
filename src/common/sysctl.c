#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "global.h"
#include "cpu.h"

uint32_t get_sys_info_by_name(char* name) {
  size_t size = 0;
  uint32_t ret = 0;

  if (sysctlbyname(name, NULL, &size, NULL, 0) != 0) {
    printWarn("sysctlbyname(%s) failed: %s", name, strerror(errno));
  }
  else if (size == sizeof(uint32_t)) {
    sysctlbyname(name, &ret, &size, NULL, 0);
  }
  else {
    printWarn("sysctl does not support non-integer lookup for '%s'", name);
  }

  return ret;
}
