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
  char* buf = malloc(sizeof(char)*DEFAULT_FILE_SIZE);
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
    #ifdef ARCH_X86
      if(hv_present) {
        printWarn("Could not open '%s'", path);
      }
      else {
        perror("open");
        printBug("Could not open '%s'", path);            
      }
    #elif ARCH_ARM
      printWarn("Could not open '%s'", path); 
    #endif
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
