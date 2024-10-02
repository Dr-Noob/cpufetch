#include <errno.h>
#include <string.h>

#include "../common/global.h"
#include "udev.h"

#define _PATH_DEVTREE          "/proc/device-tree/compatible"
#define CPUINFO_UARCH_STR      "uarch\t\t: "
#define CPUINFO_EXTENSIONS_STR "isa\t\t: "
#define CPUINFO_RISCV_MVENDORID "mvendorid\t:"
#define CPUINFO_RISCV_MARCHID   "marchid\t\t:"
#define CPUINFO_RISCV_MIMPID    "mimpid\t\t:"
#define DEVTREE_HARDWARE_FIELD 0

char* get_field_from_devtree(int DEVTREE_FIELD) {
  int filelen;
  char* buf;
  if((buf = read_file(_PATH_DEVTREE, &filelen)) == NULL) {
    printWarn("read_file: %s: %s", _PATH_DEVTREE, strerror(errno));
    return NULL;
  }

  // Here we would use strstr to find the comma.
  // However, the device-tree file may contain NULL
  // bytes in the middle of the string, which would
  // cause strstr to return NULL even when there might
  // be an occurence after the NULL byte
  //
  // We iterate the string backwards to find the field
  // in position n-DEVTREE_HARDWARE_FIELD where n
  // is the number of fields.
  int i=0;
  char* tmp1 = buf+filelen-1;
  do {
    tmp1--;
    if(*tmp1 == ',') i++;
  } while(tmp1 != buf && i <= DEVTREE_FIELD);

  if(tmp1 == buf) {
    printWarn("get_field_from_devtree: Unable to find field %d", DEVTREE_FIELD);
    return NULL;
  }

  tmp1++;
  int strlen = filelen-(tmp1-buf);
  char* hardware = ecalloc(strlen, sizeof(char));
  strncpy(hardware, tmp1, strlen-1);

  return hardware;
}

char* parse_cpuinfo_field(char* field_str) {
  int filelen;
  char* buf;
  if((buf = read_file(_PATH_CPUINFO, &filelen)) == NULL) {
    printWarn("read_file: %s: %s", _PATH_CPUINFO, strerror(errno));
    return NULL;
  }

  char* tmp = strstr(buf, field_str);
  if(tmp == NULL) {
    printWarn("parse_cpuinfo_field: Unable to find field %s", field_str);
    return NULL;
  }

  tmp += strlen(field_str);
  char* end = strstr(tmp, "\n");

  if(end == NULL) {
    printWarn("parse_cpuinfo_field: Unable to find newline after field %s", field_str);
    return NULL;
  }

  int ret_strlen = (end-tmp);
  char* ret = ecalloc(ret_strlen+1, sizeof(char));
  strncpy(ret, tmp, sizeof(char) * ret_strlen);

  return ret;
}

unsigned long parse_cpuinfo_field_uint64(char* field_str) {  
  int filelen;
  char* buf;
  if((buf = read_file(_PATH_CPUINFO, &filelen)) == NULL) {
    printWarn("read_file: %s: %s", _PATH_CPUINFO, strerror(errno));
    return 0;
  }

  char* tmp = strstr(buf, field_str);
  if(tmp == NULL) return 0;
  tmp += strlen(field_str);

  char* end;
  errno = 0;
  long ret = strtol(tmp, &end, 16);
  if(errno != 0) {
    printWarn("strtol: %s:\n", strerror(errno));
    return 0;
  }
  
  return ret;
}

// Creates and fills in the riscv_cpuinfo struct (which contains
// mvendorid, marchid and mimpid) using cpuinfo to fetch the values.
// 
// Every RISC-V hart (hardware thread) [1] provides a
// marchid (Machine Architecture ID register) CSR that encodes its
// base microarchitecture [2]. For more information about
// marchid and the rest of values, see [3].
// [1] https://groups.google.com/a/groups.riscv.org/g/sw-dev/c/QKjUDjz_vKo
// [2] https://github.com/riscv/riscv-isa-manual/blob/main/marchid.md
// [3] https://five-embeddev.com/riscv-priv-isa-manual/Priv-v1.12/machine.html#machine-architecture-id-register-marchid
struct riscv_cpuinfo *get_riscv_cpuinfo(void) {
  struct riscv_cpuinfo* ci = emalloc(sizeof(struct riscv_cpuinfo));

  ci->mvendorid = parse_cpuinfo_field_uint64(CPUINFO_RISCV_MVENDORID);
  ci->marchid = parse_cpuinfo_field_uint64(CPUINFO_RISCV_MARCHID);
  ci->mimpid = parse_cpuinfo_field_uint64(CPUINFO_RISCV_MIMPID);

  if (ci->mvendorid == 0 && ci->mvendorid == 0 && ci->mvendorid == 0)
    return NULL;

  return ci;
}

char* get_hardware_from_devtree(void) {
  return get_field_from_devtree(DEVTREE_HARDWARE_FIELD);
}

char* get_uarch_from_cpuinfo(void) {
  return parse_cpuinfo_field(CPUINFO_UARCH_STR);
}

char* get_extensions_from_cpuinfo(void) {
  return parse_cpuinfo_field(CPUINFO_EXTENSIONS_STR);
}

