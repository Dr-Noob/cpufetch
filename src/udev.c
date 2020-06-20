#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "udev.h"

#define _PATH_SYS_SYSTEM    "/sys/devices/system"
#define _PATH_SYS_CPU       _PATH_SYS_SYSTEM"/cpu"
#define _PATH_ONE_CPU       _PATH_SYS_CPU"/cpu0"

#define _PATH_FREQUENCY     _PATH_ONE_CPU"/cpufreq"
#define _PATH_FREQUENCY_MAX _PATH_FREQUENCY"/cpuinfo_max_freq"
#define _PATH_FREQUENCY_MIN _PATH_FREQUENCY"/cpuinfo_min_freq"

struct frequency {
  long max;
  long min;
};

// Parses buf which should be expressed in the way: xxxxK where 'x' are numbers and 'K' refers to kilobytes. 
// Returns the size as a int in bytes
int getSize(char* buf, int size) {
  char* end = strstr (buf,"K");
  if(end == NULL) {
    printf("ERROR in getSize(strstr)\n");
    return UNKNOWN;
  }
  *end = 0;
  int cachsize = atoi(buf);
  if(cachsize == 0) {
    printf("ERROR in getSize(atoi)\n");
    return UNKNOWN;
  }
  return cachsize;
}

// Returns CPU frequency in Hz
long get_freq_from_file(char* path) {
  FILE *file = fopen(path, "r");

  if(file == NULL) {
    //Doest not exist
    return UNKNOWN;
  }

  //File exists, read it
  int fd = fileno(file);
  int bytes_read = 0;
  int offset = 0;
  int block = 1;
  char* buf = malloc(sizeof(char)*DEFAULT_FILE_SIZE);
  memset(buf, 0, sizeof(char)*DEFAULT_FILE_SIZE);

  while (  (bytes_read = read(fd, buf+offset, block)) > 0 ) {
    offset += bytes_read;
  }

  int ret = atoi(buf);
  free(buf);
  if(ret == 0) {
    printf("error in get_freq_from_file\n");
    return UNKNOWN;
  }
  fclose(file);
  return (long)ret*1000;
}

long get_freq(struct frequency* freq) {
  return freq->max;
}

char* get_str_freq(struct frequency* freq) {
  //Max 3 digits and 3 for '(M/G)Hz' plus 1 for '\0'
  unsigned int size = (4+3+1);
  assert(strlen(STRING_UNKNOWN)+1 <= size);
  char* string = malloc(sizeof(char)*size);
  if(freq->max == UNKNOWN)
    snprintf(string,strlen(STRING_UNKNOWN)+1,STRING_UNKNOWN);
  else if(freq->max >= 1000000000)
    snprintf(string,size,"%.2f"STRING_GIGAHERZ,(float)(freq->max)/1000000000);
  else
    snprintf(string,size,"%.2f"STRING_MEGAHERZ,(float)(freq->max)/1000000);
  return string;
}

struct frequency* get_frequency_info() {
  struct frequency* freq = malloc(sizeof(struct frequency));
  
  freq->max = get_freq_from_file(_PATH_FREQUENCY_MAX);
  freq->min = get_freq_from_file(_PATH_FREQUENCY_MIN);
  
  return freq;
}

void free_freq_struct(struct frequency* freq) {
  free(freq);
}

void debugFrequency(struct frequency* freq) {
  printf("max f=%ldMhz\n",freq->max);
  printf("min f=%ldMhz\n",freq->min);
}
