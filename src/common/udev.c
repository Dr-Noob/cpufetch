#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "udev.h"
#include "global.h"

#ifdef ARCH_X86
  #include "../x86/cpuid.h"
#else
  #include "../arm/midr.h"
#endif

#define _PATH_SYS_SYSTEM        "/sys/devices/system"
#define _PATH_SYS_CPU           "/cpu"
#define _PATH_FREQUENCY         "/cpufreq"
#define _PATH_FREQUENCY_MAX     "/cpuinfo_max_freq"
#define _PATH_FREQUENCY_MIN     "/cpuinfo_min_freq"

#define _PATH_FREQUENCY_MAX_LEN 100
#define DEFAULT_FILE_SIZE       4096

long get_freq_from_file(char* path) {
  int fd = open(path, O_RDONLY);

  if(fd == -1) {    
  #ifdef ARCH_X86
    perror("open");
    printBug("Could not open '%s'", path);
  #elif ARCH_ARM
    printWarn("Could not open '%s'", path);
  #endif
    return UNKNOWN_FREQ;
  }

  //File exists, read it
  int bytes_read = 0;
  int offset = 0;
  int block = 1;
  char* buf = malloc(sizeof(char)*DEFAULT_FILE_SIZE);
  memset(buf, 0, sizeof(char)*DEFAULT_FILE_SIZE);

  while (  (bytes_read = read(fd, buf+offset, block)) > 0 ) {
    offset += bytes_read;
  }

  char* end;
  errno = 0;
  long ret = strtol(buf, &end, 10);
  if(errno != 0) {
    perror("strtol");
    printBug("Failed parsing '%s' file. Read data was: '%s'", path, buf);
    free(buf);
    return UNKNOWN_FREQ;
  }
  
  // We will be getting the frequency in KHz
  // We consider it is an error if frequency is
  // greater than 10 GHz or less than 100 MHz
  if(ret > 10000 * 1000 || ret <  100 * 1000) {
    printBug("Invalid data was read from file '%s': %ld\n", path, ret);
    return UNKNOWN_FREQ;
  }
  
  free(buf);
  if (close(fd) == -1) {
    perror("close");
    printErr("Closing '%s' failed\n", path);    
  }
  
  return ret/1000;
}

long get_max_freq_from_file(uint32_t core) {
  char path[_PATH_FREQUENCY_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s", _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_FREQUENCY, _PATH_FREQUENCY_MAX);
  return get_freq_from_file(path);
}

long get_min_freq_from_file(uint32_t core) {
  char path[_PATH_FREQUENCY_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s", _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_FREQUENCY, _PATH_FREQUENCY_MIN);
  return get_freq_from_file(path);
}

#ifdef ARCH_ARM

#define _PATH_CPUS_PRESENT      _PATH_SYS_SYSTEM _PATH_SYS_CPU "/present"
#define _PATH_CPUINFO "/proc/cpuinfo"

#define CPUINFO_CPU_IMPLEMENTER_STR  "CPU implementer\t: "
#define CPUINFO_CPU_ARCHITECTURE_STR "CPU architecture: "
#define CPUINFO_CPU_VARIANT_STR      "CPU variant\t: "
#define CPUINFO_CPU_PART_STR         "CPU part\t: "
#define CPUINFO_CPU_REVISION_STR     "CPU revision\t: "
#define CPUINFO_HARDWARE_STR         "Hardware\t: "

#define CPUINFO_CPU_STRING "processor"

// https://www.kernel.org/doc/html/latest/core-api/cpu_hotplug.html
int get_ncores_from_cpuinfo() {
  // Examples:
  // 0-271
  // 0-5
  // 0-7
  int fd = open(_PATH_CPUS_PRESENT, O_RDONLY);
  
  if(fd == -1) {
    fprintf(stderr, "%s: ", _PATH_CPUS_PRESENT);      
    perror("open");
    return UNKNOWN;
  }

  //File exists, read it
  int bytes_read = 0;
  int offset = 0;
  int block = 128;
  char* buf = malloc(sizeof(char)*DEFAULT_FILE_SIZE);
  memset(buf, 0, sizeof(char)*DEFAULT_FILE_SIZE);

  while (  (bytes_read = read(fd, buf+offset, block)) > 0 ) {
    offset += bytes_read;
  }

  int ncores = 0;
  char* tmp1 = strstr(buf, "-") + 1;
  char* tmp2 = strstr(buf, "\n");
  char ncores_str[offset];
  memcpy(ncores_str, tmp1, tmp2-tmp1);
  
  char* end;
  errno = 0;
  ncores = strtol(ncores_str, &end, 10) + 1;  
  if(errno != 0) {
    perror("strtol");
    return UNKNOWN;
  }

  free(buf);

  if (close(fd) == -1) {
    perror("close");
  }

  return ncores;
}

long parse_cpuinfo_field(char* buf, char* field_str, int field_base) {
  char* tmp = strstr(buf, field_str);
  if(tmp == NULL) return -1;
  tmp += strlen(field_str);

  char* end;
  errno = 0;
  long ret = strtol(tmp, &end, field_base);
  if(errno != 0) {
    perror("strtol");
    return -1;
  }

  return ret;
}
// https://developer.arm.com/docs/ddi0595/h/aarch32-system-registers/midr
// https://static.docs.arm.com/ddi0595/h/SysReg_xml_v86A-2020-06.pdf
uint32_t get_midr_from_cpuinfo(uint32_t core, bool* success) {
  int fd = open(_PATH_CPUINFO, O_RDONLY);
  *success = true;
  
  if(fd == -1) {
    *success = false;
    perror("open");
    return 0;
  }

  //File exists, read it
  int bytes_read = 0;
  int offset = 0;
  int block = 128;
  char* buf = malloc(sizeof(char)*DEFAULT_FILE_SIZE);
  memset(buf, 0, sizeof(char)*DEFAULT_FILE_SIZE);

  while (  (bytes_read = read(fd, buf+offset, block)) > 0 ) {
    offset += bytes_read;
  }

  char* tmp = buf;
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
    printf("Failed parsing cpu_implementer\n");
    *success = false;    
    return 0;
  }
  cpu_implementer = (uint32_t) ret;

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_ARCHITECTURE_STR, 10)) < 0) {
    printf("Failed parsing cpu_architecture\n");
    *success = false;    
    return 0;
  }
  cpu_architecture = (uint32_t) 0xF; // Why?

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_VARIANT_STR, 16)) < 0) {
    printf("Failed parsing cpu_variant\n");
    *success = false;    
    return 0;
  }
  cpu_variant = (uint32_t) ret;

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_PART_STR, 16)) < 0) {
    printf("Failed parsing cpu_part\n");
    *success = false;    
    return 0;
  }
  cpu_part = (uint32_t) ret;

  if ((ret = parse_cpuinfo_field(tmp, CPUINFO_CPU_REVISION_STR, 10)) < 0) {
    printf("Failed parsing cpu_revision\n");
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

char* get_hardware_from_cpuinfo() {
  int fd = open(_PATH_CPUINFO, O_RDONLY);
  
  if(fd == -1) {
    perror("open");
    return NULL;
  }

  //File exists, read it
  int bytes_read = 0;
  int offset = 0;
  int block = 128;
  char* buf = malloc(sizeof(char)*DEFAULT_FILE_SIZE);
  memset(buf, 0, sizeof(char)*DEFAULT_FILE_SIZE);

  while (  (bytes_read = read(fd, buf+offset, block)) > 0 ) {
    offset += bytes_read;
  }   
  
  char* tmp1 = strstr(buf, CPUINFO_HARDWARE_STR);
  if(tmp1 == NULL) return NULL;
  tmp1 = tmp1 + strlen(CPUINFO_HARDWARE_STR);
  char* tmp2 = strstr(tmp1, "\n");
  
  int strlen = (1 + (tmp2-tmp1));
  char* hardware = malloc(sizeof(char) * strlen);
  memset(hardware, 0, sizeof(char) * strlen);
  strncpy(hardware, tmp1, tmp2-tmp1);
  
  return hardware;
}

#endif /* ARCH_ARM */
