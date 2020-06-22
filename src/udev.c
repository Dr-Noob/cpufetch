#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "global.h"
#include "standart.h"

#define _PATH_SYS_SYSTEM    "/sys/devices/system"
#define _PATH_SYS_CPU       _PATH_SYS_SYSTEM"/cpu"
#define _PATH_ONE_CPU       _PATH_SYS_CPU"/cpu0"

#define _PATH_FREQUENCY     _PATH_ONE_CPU"/cpufreq"
#define _PATH_FREQUENCY_MAX _PATH_FREQUENCY"/cpuinfo_max_freq"
#define _PATH_FREQUENCY_MIN _PATH_FREQUENCY"/cpuinfo_min_freq"

#define DEFAULT_FILE_SIZE 4096

long get_freq_from_file(char* path) {
  int fd = open(path, O_RDONLY);

  if(fd == -1) {
    perror("open");
    printBug("Could not open '%s'", path);
    return UNKNOWN;
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
    return UNKNOWN;
  }
  
  // We will be getting the frequency in KHz
  // We consider it is an error if frequency is
  // greater than 10 GHz or less than 100 MHz
  if(ret > 10000 * 1000 || ret <  100 * 1000) {
    printBug("Invalid data was read from file '%s': %ld\n", path, ret);
    return UNKNOWN;
  }
  
  free(buf);
  if (close(fd) == -1) {
    perror("close");
    printErr("Closing '%s' failed\n", path);    
  }
  
  return ret/1000;
}

long get_max_freq_from_file() {
  return get_freq_from_file(_PATH_FREQUENCY_MAX);
}

long get_min_freq_from_file() {
  return get_freq_from_file(_PATH_FREQUENCY_MIN);
}
