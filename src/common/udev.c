#include "../common/global.h"
#include "udev.h"
#include "global.h"
#include "cpu.h"

#define _PATH_DEVTREE          "/proc/device-tree/compatible"

// https://www.kernel.org/doc/html/latest/core-api/cpu_hotplug.html
int get_ncores_from_cpuinfo(void) {
  // Examples:
  // 0-271
  // 0-7
  // 0

  int filelen;
  char* buf;
  if((buf = read_file(_PATH_CPUS_PRESENT, &filelen)) == NULL) {
    printWarn("read_file: %s: %s\n", _PATH_CPUS_PRESENT, strerror(errno));
    return -1;
  }

  int ncores;
  char* tmp1;
  if((tmp1 = strstr(buf, "-")) == NULL) {
    // file contains no - character, we assume that it contains 0,
    // which means that the CPU contains only one core
    return 1;
  }
  else {
    tmp1++;
  }
  char* tmp2 = strstr(buf, "\n");
  char ncores_str[filelen];
  memset(ncores_str, 0, sizeof(char) * filelen);
  memcpy(ncores_str, tmp1, tmp2-tmp1);

  char* end;
  errno = 0;
  ncores = strtol(ncores_str, &end, 10) + 1;
  if(errno != 0) {
    printWarn("strtol: %s:\n", strerror(errno));
    return -1;
  }

  free(buf);

  return ncores;
}

char* read_file(char* path, int* len) {
  int fd = open(path, O_RDONLY);

  if(fd == -1) {
    return NULL;
  }

  //File exists, read it
  int bytes_read = 0;
  int offset = 0;
  int block = 1024;
  int buf_size = block * 4;
  char* buf = emalloc(sizeof(char) * buf_size);

  while ((bytes_read = read(fd, buf+offset, block)) > 0) {
    offset += bytes_read;
    if(offset + block > buf_size) {
      buf = erealloc(buf, sizeof(char) * (buf_size + block));
      buf_size += block;
    }
  }
  buf[offset] = '\0';

  if (close(fd) == -1) {
    return NULL;
  }

  *len = offset;
  return buf;
}

long get_freq_from_file(char* path) {
  int filelen;
  char* buf;
  if((buf = read_file(path, &filelen)) == NULL) {
    printWarn("Could not open '%s'", path);
    return UNKNOWN_DATA;
  }

  char* end;
  errno = 0;
  long ret = strtol(buf, &end, 10);
  if(errno != 0) {
    printBug("strtol: %s", strerror(errno));
    free(buf);
    return UNKNOWN_DATA;
  }

  // We will be getting the frequency in KHz
  // We consider it is an error if frequency is
  // greater than 10 GHz or less than 100 MHz
  if(ret > 10000 * 1000 || ret <  100 * 1000) {
    printBug("Invalid data was read from file '%s': %ld\n", path, ret);
    return UNKNOWN_DATA;
  }

  free(buf);

  return ret/1000;
}

long get_cache_size_from_file(char* path) {
  int filelen;
  char* buf;
  if((buf = read_file(path, &filelen)) == NULL) {
    printWarn("Could not open '%s'", path);
    return -1;
  }

  buf[filelen] = '\0'; // remove the K at the end

  char* end;
  errno = 0;
  long ret = strtol(buf, &end, 10);
  if(errno != 0) {
    printBug("strtol: %s", strerror(errno));
    free(buf);
    return -1;
  }

  free(buf);

  return ret * 1024;
}

char* get_field_from_cpuinfo(char* CPUINFO_FIELD) {
  int filelen;
  char* buf;
  if((buf = read_file(_PATH_CPUINFO, &filelen)) == NULL) {
    printWarn("read_file: %s: %s:\n", _PATH_CPUINFO, strerror(errno));
    return NULL;
  }

  char* tmp1 = strstr(buf, CPUINFO_FIELD);
  if(tmp1 == NULL) return NULL;
  tmp1 = tmp1 + strlen(CPUINFO_FIELD);
  char* tmp2 = strstr(tmp1, "\n");

  int strlen = (1 + (tmp2-tmp1));
  char* hardware = ecalloc(strlen, sizeof(char));
  strncpy(hardware, tmp1, tmp2-tmp1);

  return hardware;
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

long get_l1i_cache_size(uint32_t core) {
  char path[_PATH_CACHE_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s",  _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_CACHE_L1I, _PATH_CACHE_SIZE);
  return get_cache_size_from_file(path);
}

long get_l1d_cache_size(uint32_t core) {
  char path[_PATH_CACHE_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s",  _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_CACHE_L1D, _PATH_CACHE_SIZE);
  return get_cache_size_from_file(path);
}

long get_l2_cache_size(uint32_t core) {
  char path[_PATH_CACHE_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s",  _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_CACHE_L2, _PATH_CACHE_SIZE);
  return get_cache_size_from_file(path);
}

long get_l3_cache_size(uint32_t core) {
  char path[_PATH_CACHE_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s",  _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_CACHE_L3, _PATH_CACHE_SIZE);
  return get_cache_size_from_file(path);
}

void add_shared_map(uint32_t** src, int src_idx, uint32_t** dst, int dst_idx, int n) {
  for(int j=0; j < n; j++) {
    dst[dst_idx][j] = src[src_idx][j];
  }
}

bool maps_equal(uint32_t* map1, uint32_t* map2, int n) {
  for(int i=0; i < n; i++) {
    if(map1[i] != map2[i]) return false;
  }
  return true;
}

// Generic function to count the number of distinct
// elements in the list of files passed in char** paths.
// An element can be potentially anything.
// We use this function to count:
// - The number of caches
// - The number of sockets
int get_num_elements_from_files(char** paths, int num_paths) {
  int filelen;
  char* buf;
  char* tmpbuf;

  // 1. Count the number of bitmasks per file
  if((buf = read_file(paths[0], &filelen)) == NULL) {
    printWarn("Could not open '%s'", paths[0]);
    return -1;
  }
  int num_bitmasks = 1;
  for(int i=0; buf[i]; i++) {
    num_bitmasks += (buf[i] == ',');
  }

  // 2. Read map from every core
  uint32_t** maps = emalloc(sizeof(uint32_t *) * num_paths);
  for(int i=0; i < num_paths; i++) {
    maps[i] = emalloc(sizeof(uint32_t) * num_bitmasks);

    if((buf = read_file(paths[i], &filelen)) == NULL) {
      printWarn("Could not open '%s'", paths[i]);
      return -1;
    }

    for(int j=0; j < num_bitmasks; j++) {
      char* end;
      tmpbuf = emalloc(sizeof(char) * (strlen(buf) + 1));
      memset(tmpbuf, 0, sizeof(char) * (strlen(buf) + 1));
      char* commaend = strstr(buf, ",");
      if(commaend == NULL) {
        strcpy(tmpbuf, buf);
      }
      else {
        strncpy(tmpbuf, buf, commaend-buf);
      }
      errno = 0;
      long ret = strtol(tmpbuf, &end, 16);
      if(errno != 0) {
        printf("strtol: %s", strerror(errno));
        free(buf);
        return -1;
      }

      maps[i][j] = (uint32_t) ret;
      buf = commaend + 1;
      free(tmpbuf);
    }
  }

  // 2. Count number of different masks; this is the number of elements
  int num_elements = 0;
  bool found = false;
  uint32_t** unique_maps = emalloc(sizeof(uint32_t *) * num_paths);
  for(int i=0; i < num_paths; i++) {
    unique_maps[i] = emalloc(sizeof(uint32_t) * num_bitmasks);
    for(int j=0; j < num_bitmasks; j++) {
      unique_maps[i][j] = 0;
    }
  }

  for(int i=0; i < num_paths; i++) {
    for(int j=0; j < num_paths && !found; j++) {
      if(maps_equal(maps[i], unique_maps[j], num_bitmasks)) found = true;
    }
    if(!found) {
      add_shared_map(maps, i, unique_maps, num_elements, num_bitmasks);
      num_elements++;
    }
    found = false;
  }

  return num_elements;
}

int get_num_caches_from_files(char** paths, int num_paths) {
  return get_num_elements_from_files(paths, num_paths);
}

int get_num_sockets_from_files(char** paths, int num_paths) {
  return get_num_elements_from_files(paths, num_paths);
}

int get_num_caches_by_level(struct cpuInfo* cpu, uint32_t level) {
  char** paths = emalloc(sizeof(char *) * cpu->topo->total_cores);
  char* cache_path = NULL;

  if(level == 0) cache_path = _PATH_CACHE_L1I;
  else if(level == 1) cache_path = _PATH_CACHE_L1D;
  else if(level == 2) cache_path = _PATH_CACHE_L2;
  else if(level == 3) cache_path = _PATH_CACHE_L3;
  else {
    printBug("Found invalid cache level to inspect: %d\n", level);
    return -1;
  }

  for(int i=0; i < cpu->topo->total_cores; i++) {
    paths[i] = emalloc(sizeof(char) * _PATH_CACHE_MAX_LEN);
    sprintf(paths[i], "%s%s/cpu%d%s%s",  _PATH_SYS_SYSTEM, _PATH_SYS_CPU, i, cache_path, _PATH_CACHE_SHARED_MAP);
  }

  int ret = get_num_caches_from_files(paths, cpu->topo->total_cores);

  for(int i=0; i < cpu->topo->total_cores; i++)
    free(paths[i]);
  free(paths);

  return ret;
}

int get_num_sockets_package_cpus(struct topology* topo) {
  // Get number of sockets using
  // /sys/devices/system/cpu/cpu*/topology/package_cpus

  char** paths = emalloc(sizeof(char *) * topo->total_cores);

  for(int i=0; i < topo->total_cores; i++) {
    paths[i] = emalloc(sizeof(char) * _PATH_PACKAGE_MAX_LEN);
    sprintf(paths[i], "%s%s/cpu%d%s",  _PATH_SYS_SYSTEM, _PATH_SYS_CPU, i, _PATH_TOPO_PACKAGE_CPUS);
  }

  int ret = get_num_sockets_from_files(paths, topo->total_cores);

  for(int i=0; i < topo->total_cores; i++)
    free(paths[i]);
  free(paths);

  return ret;
}

// Inspired in is_devtree_compatible from lscpu
bool is_devtree_compatible(char* str) {
  int filelen;
  char* buf;
  if((buf = read_file("/proc/device-tree/compatible", &filelen)) == NULL) {
    return false;
  }

  char* tmp;
  if((tmp = strstr(buf, str)) == NULL) {
    return false;
  }
  return true;
}

char* get_devtree_compatible(int *filelen) {
  char* buf;

  if ((buf = read_file(_PATH_DEVTREE, filelen)) == NULL) {
    printWarn("read_file: %s: %s", _PATH_DEVTREE, strerror(errno));
  }

  return buf;
}

// Returns a list of structs devtree, each containing both the vendor and the
// model, coming from the compatible file from the device tree. In this
// context, vendor refers to the first string of every entry and the model to
// the second. For instance, given a compatible file with:
// "str1,foo1.str2,foo2" (where . denotes the NULL byte, i.e., the separator),
// then this function will return a list with two structs, the first one
// containing str1 and foo1 and the other containing str2 and foo2.
struct devtree** get_devtree_compatible_struct(int *num_vendors_ptr) {
  int len;
  char* dt = get_devtree_compatible(&len);
  if (dt == NULL) {
    return NULL;
  }

  int num_vendors = 0;
  char* ptr = dt;

  for (int ptrpos = 0; ptrpos < len; ptrpos = (ptr-dt)) {
    ptr = memchr(ptr, '\0', len);
    if (ptr == NULL) {
      printBug("get_devtree_compatible_struct: Unable to find delimiter (1) (num_vendors=%d)", num_vendors);
      return NULL;
    }
    ptr++;
    num_vendors++;
  }

  struct devtree** vendors = emalloc(sizeof(struct devtree *) * num_vendors);
  ptr = dt;

  for (int ptrpos = 0, i = 0; ptrpos < len; ptrpos = (ptr-dt), i++) {
    char* comma_ptr = strstr(ptr, ",");
    if (comma_ptr == NULL) {
      printBug("get_devtree_compatible_struct: Unable to find comma (num_vendors=%d)", num_vendors);
      return NULL;
    }
    comma_ptr = comma_ptr-1; // Point right before comma

    char* end_ptr = memchr(comma_ptr, '\0', len - ptrpos);
    if (end_ptr == NULL) {
      printBug("get_devtree_compatible_struct: Unable to find delimiter (2) (num_vendors=%d)", num_vendors);
      return NULL;
    }

    int vendor_str_len = (comma_ptr-ptr)+1;
    int model_str_len = (end_ptr-(comma_ptr+2))+1;

    vendors[i] = emalloc(sizeof(struct devtree));
    vendors[i]->vendor = ecalloc(vendor_str_len, sizeof(char));
    vendors[i]->model = ecalloc(model_str_len, sizeof(char));

    strncpy(vendors[i]->vendor, ptr, vendor_str_len);
    strncpy(vendors[i]->model, comma_ptr+2, model_str_len);

    ptr = memchr(ptr, '\0', len);
    if (ptr == NULL) {
      printBug("get_devtree_compatible_struct: Unable to find delimiter (3) (num_vendors=%d)", num_vendors);
      return NULL;
    }
    ptr++; // Point right after delimiter
  }

  *num_vendors_ptr = num_vendors;
  return vendors;
}
