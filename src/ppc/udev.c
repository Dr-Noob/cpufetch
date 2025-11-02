#include <errno.h>

#include "../common/udev.h"
#include "../common/global.h"
#include "udev.h"

#define _PATH_TOPO_CORE_ID         "topology/core_id"
#define _PATH_TOPO_PACKAGE_ID      "topology/physical_package_id"
#define CPUINFO_FREQUENCY_STR      "clock\t\t: "

bool fill_array_from_sys(int *core_ids, int total_cores, char* SYS_PATH) {
  int filelen;
  char* buf;
  char* end;
  char path[128];
  memset(path, 0, sizeof(char) * 128);

  for(int i=0; i < total_cores; i++) {
    sprintf(path, "%s%s/cpu%d/%s", _PATH_SYS_SYSTEM, _PATH_SYS_CPU, i, SYS_PATH);
    if((buf = read_file(path, &filelen)) == NULL) {
      printWarn("fill_array_from_sys: %s: %s", path, strerror(errno));
      return false;
    }

    errno = 0;
    core_ids[i] = strtol(buf, &end, 10);
    if(errno != 0) {
      printWarn("fill_array_from_sys: %s:", strerror(errno));
      return false;
    }
    free(buf);
  }

  return true;
}

bool fill_core_ids_from_sys(int *core_ids, int total_cores) {
  return fill_array_from_sys(core_ids, total_cores, _PATH_TOPO_CORE_ID);
}

bool fill_package_ids_from_sys(int* package_ids, int total_cores) {
  bool status = fill_array_from_sys(package_ids, total_cores, _PATH_TOPO_PACKAGE_ID);
  if(status) {
    // fill_array_from_sys completed successfully, but we
    // must to check the integrity of the package_ids array
    for(int i=0; i < total_cores; i++) {
      if(package_ids[i] == -1) {
        printWarn("fill_package_ids_from_sys: package_ids[%d] = -1", i);
        return false;
      }
      else if(package_ids[i] >= total_cores || package_ids[i] < 0) {
        printBug("fill_package_ids_from_sys: package_ids[%d] = %d", i, package_ids[i]);
        return false;
      }
    }

    return true;
  }
  return false;
}

long get_frequency_from_cpuinfo(void) {
  char* freq_str = get_field_from_cpuinfo(CPUINFO_FREQUENCY_STR);
  if(freq_str == NULL) {
    return UNKNOWN_DATA;
  }
  else {
    // freq_str should be in the form XXXX.YYYYYYMHz
    char* dot = strstr(freq_str, ".");
    freq_str[dot-freq_str] = '\0';

    char* end;
    errno = 0;
    long ret = strtol(freq_str, &end, 10);
    if(errno != 0) {
      printBug("strtol: %s", strerror(errno));
      free(freq_str);
      return UNKNOWN_DATA;
    }

    // We consider it an error if frequency is
    // greater than 10 GHz or less than 100 MHz
    if(ret > 10000 || ret <  100) {
      printBug("Invalid data was read from file '%s': %ld\n", CPUINFO_FREQUENCY_STR, ret);
      return UNKNOWN_DATA;
    }

    return ret;
  }
}
