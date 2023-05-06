#include "../common/global.h"
#include "udev.h"
#include "midr.h"

#define _PATH_DEVICETREE_MODEL       "/sys/firmware/devicetree/base/model"

#define CPUINFO_CPU_IMPLEMENTER_STR  "CPU implementer\t: "
#define CPUINFO_CPU_ARCHITECTURE_STR "CPU architecture: "
#define CPUINFO_CPU_VARIANT_STR      "CPU variant\t: "
#define CPUINFO_CPU_PART_STR         "CPU part\t: "
#define CPUINFO_CPU_REVISION_STR     "CPU revision\t: "
#define CPUINFO_HARDWARE_STR         "Hardware\t: "
#define CPUINFO_REVISION_STR         "Revision\t: "

#define CPUINFO_CPU_STRING "processor"

long parse_cpuinfo_field(char* buf, char* field_str, int field_base) {
  char* tmp = strstr(buf, field_str);
  if(tmp == NULL) return -1;
  tmp += strlen(field_str);

  char* end;
  errno = 0;
  long ret = strtol(tmp, &end, field_base);
  if(errno != 0) {
    printWarn("strtol: %s:\n", strerror(errno));
    return -1;
  }

  return ret;
}
// https://developer.arm.com/docs/ddi0595/h/aarch32-system-registers/midr
// https://static.docs.arm.com/ddi0595/h/SysReg_xml_v86A-2020-06.pdf
uint32_t get_midr_from_cpuinfo(uint32_t core, bool* success) {
  int filelen;
  char* buf;
  *success = true;
  if((buf = read_file(_PATH_CPUINFO, &filelen)) == NULL) {
    printWarn("read_file: %s: %s\n", _PATH_CPUINFO, strerror(errno));
    *success = false;
    return 0;
  }

  char* tmp = strstr(buf, CPUINFO_CPU_STRING);
  uint32_t current_core = 0;
  while(core != current_core && tmp != NULL) {
    tmp++;
    current_core++;
    tmp = strstr(tmp, CPUINFO_CPU_STRING);
  }

  if(tmp == NULL) {
    *success = false;
    return 0;
  }

  uint32_t cpu_implementer;
  uint32_t cpu_architecture;
  uint32_t cpu_variant;
  uint32_t cpu_part;
  uint32_t cpu_revision;
  uint32_t midr = 0;
  long ret;

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_IMPLEMENTER_STR, 16)) < 0) {
    printBug("get_midr_from_cpuinfo: Failed parsing cpu_implementer\n");
    *success = false;
    return 0;
  }
  cpu_implementer = (uint32_t) ret;

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_ARCHITECTURE_STR, 10)) < 0) {
    printBug("get_midr_from_cpuinfo: Failed parsing cpu_architecture\n");
    *success = false;
    return 0;
  }
  cpu_architecture = (uint32_t) 0xF; // Why?

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_VARIANT_STR, 16)) < 0) {
    printBug("get_midr_from_cpuinfo: Failed parsing cpu_variant\n");
    *success = false;
    return 0;
  }
  cpu_variant = (uint32_t) ret;

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_PART_STR, 16)) < 0) {
    printBug("get_midr_from_cpuinfo: Failed parsing cpu_part\n");
    *success = false;
    return 0;
  }
  cpu_part = (uint32_t) ret;

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_REVISION_STR, 10)) < 0) {
    printBug("get_midr_from_cpuinfo: Failed parsing cpu_revision\n");
    *success = false;
    return 0;
  }
  cpu_revision = (uint32_t) ret;

  midr = midr_set_implementer(midr, cpu_implementer);
  midr = midr_set_variant(midr, cpu_variant);
  midr = midr_set_architecture(midr, cpu_architecture);
  midr = midr_set_part(midr, cpu_part);
  midr = midr_set_revision(midr, cpu_revision);

  return midr;
}

char* get_hardware_from_cpuinfo(void) {
  return get_field_from_cpuinfo(CPUINFO_HARDWARE_STR);
}

char* get_revision_from_cpuinfo(void) {
  return get_field_from_cpuinfo(CPUINFO_REVISION_STR);
}

bool is_raspberry_pi(void) {
  int filelen;
  char* buf;
  if((buf = read_file(_PATH_DEVICETREE_MODEL, &filelen)) == NULL) {
    return false;
  }

  char* tmp;
  if((tmp = strstr(buf, "Raspberry Pi")) == NULL) {
    return false;
  }
  return true;
}
