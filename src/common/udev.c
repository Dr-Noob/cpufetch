#include "udev.h"
#include "global.h"
#include "cpu.h"

char* read_file(char* path, int* len) {
  int fd = open(path, O_RDONLY);

  if(fd == -1) {
    return NULL;
  }

  //File exists, read it
  int bytes_read = 0;
  int offset = 0;
  int block = 128;
  char* buf = emalloc(sizeof(char)*DEFAULT_FILE_SIZE);
  memset(buf, 0, sizeof(char)*DEFAULT_FILE_SIZE);

  while (  (bytes_read = read(fd, buf+offset, block)) > 0 ) {
    offset += bytes_read;
  }

  if (close(fd) == -1) {
    return NULL;
  }

  *len = offset;
  return buf;
}

long get_freq_from_file(char* path, bool hv_present) {
  int filelen;
  char* buf;
  if((buf = read_file(path, &filelen)) == NULL) {
    if(hv_present)
      printWarn("Could not open '%s' (HV is present)", path);
    else
      printWarn("Could not open '%s'", path);

    return UNKNOWN_FREQ;
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
    perror("strtol");
    printBug("Failed parsing '%s' file. Read data was: '%s'", path, buf);
    free(buf);
    return -1;
  }

  free(buf);

  return ret * 1024;
}

long get_max_freq_from_file(uint32_t core, bool hv_present) {
  char path[_PATH_FREQUENCY_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s", _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_FREQUENCY, _PATH_FREQUENCY_MAX);
  return get_freq_from_file(path, hv_present);
}

long get_min_freq_from_file(uint32_t core, bool hv_present) {
  char path[_PATH_FREQUENCY_MAX_LEN];
  sprintf(path, "%s%s/cpu%d%s%s", _PATH_SYS_SYSTEM, _PATH_SYS_CPU, core, _PATH_FREQUENCY, _PATH_FREQUENCY_MIN);
  return get_freq_from_file(path, hv_present);
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

int get_num_caches_from_files(char** paths, int num_paths) {
  int SHARED_MAP_MAX_LEN = 8 + 1;
  int filelen;
  char* buf;
  uint32_t* shared_maps = emalloc(sizeof(uint32_t *) * num_paths);

  // 1. Read cpu_shared_map from every core
  for(int i=0; i < num_paths; i++) {
    if((buf = read_file(paths[i], &filelen)) == NULL) {
      printWarn("Could not open '%s'", paths[i]);
      return -1;
    }

    if(filelen > SHARED_MAP_MAX_LEN) {
      printBug("Shared map length is %d while the max is be %d", filelen, SHARED_MAP_MAX_LEN);
      return -1;
    }

    char* end;
    errno = 0;
    long ret = strtol(buf, &end, 16);
    if(errno != 0) {
      perror("strtol");
      printBug("Failed parsing '%s' file. Read data was: '%s'", paths[i], buf);
      free(buf);
      return -1;
    }

    shared_maps[i] = (uint32_t) ret;
  }

  // 2. Count number of different masks; this is the number of caches
  int num_caches = 0;
  bool found = false;
  uint32_t* unique_shared_maps = emalloc(sizeof(uint32_t *) * num_paths);
  for(int i=0; i < num_paths; i++) unique_shared_maps[i] = 0;

  for(int i=0; i < num_paths; i++) {
    for(int j=0; j < num_paths && !found; j++) {
      if(shared_maps[i] == unique_shared_maps[j]) found = true;
    }
    if(!found) {
      unique_shared_maps[num_caches] = shared_maps[i];
      num_caches++;
    }
    found = false;
  }

  return num_caches;
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
