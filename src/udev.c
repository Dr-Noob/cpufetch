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

#define _PATH_CPU_CACHE     _PATH_ONE_CPU"/cache"
#define _PATH_CACHE_L1d     _PATH_CPU_CACHE"/index0/size"
#define _PATH_CACHE_L1i     _PATH_CPU_CACHE"/index1/size"
#define _PATH_CACHE_L2      _PATH_CPU_CACHE"/index2/size"
#define _PATH_CACHE_L3      _PATH_CPU_CACHE"/index3/size"

struct cache {
  int L1i;
  int L1d;
  int L2;
  int L3;
};

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

// Returns size(in bytes) of cache described by path or UNKNOWN if the cache doest no exists
//TODO: We use reads in various places, refactor
int getCache(char* path) {
  FILE *file = fopen(path, "r");

  if(file == NULL) {
    //Doest not exist
    return UNKNOWN;
  }

  //File exists, read it
  int fd = fileno(file);
  int bytes_read = 0;
  int offset = 0;
  int block = DEFAULT_BLOCK_SIZE;
  char* buf = malloc(sizeof(char)*DEFAULT_FILE_SIZE);
  memset(buf, 0, sizeof(char)*DEFAULT_FILE_SIZE);

  do {
    bytes_read = read(fd, buf+offset, block);
    offset += bytes_read;
  } while(bytes_read > 0);

  //Move size from kb to bytes
  int ret = getSize(buf,offset)*1024;
  free(buf);
  fclose(file);
  
  return ret;
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

// String functions
char* get_str_l1(struct cache* cach) {
  //Max 2 digits,2 for 'KB',6 for '(Data)'
  //and 14 for '(Instructions)'
  int size = (2*(2+2)+6+14+1);
  char* string = malloc(sizeof(char)*size);
  snprintf(string,size,"%d"STRING_KILOBYTES"(Data)%d"STRING_KILOBYTES"(Instructions)",cach->L1d/1024,cach->L1i/1024);
  return string;
}

char* get_str_l2(struct cache* cach) {
  if(cach->L2 == UNKNOWN) {
    char* string = malloc(sizeof(char)*5);
    snprintf(string,5,STRING_NONE);
    return string;
  }
  else {
    //Max 4 digits and 2 for 'KB'
    int size = (4+2+1);
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%d"STRING_KILOBYTES,cach->L2/1024);
    return string;
  }
}

char* get_str_l3(struct cache* cach) {
  if(cach->L3 == UNKNOWN) {
    char* string = malloc(sizeof(char)*5);
    snprintf(string,5,STRING_NONE);
    return string;
  }
  else {
    //Max 4 digits and 2 for 'KB'
    int size = (4+2+1);
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%d"STRING_KILOBYTES,cach->L3/1024);
    return string;
  }
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

struct cache* get_cache_info() {
  struct cache* cach = malloc(sizeof(struct cache));
  
  cach->L1i = getCache(_PATH_CACHE_L1i);
  cach->L1d = getCache(_PATH_CACHE_L1d);
  cach->L2 = getCache(_PATH_CACHE_L2);
  cach->L3 = getCache(_PATH_CACHE_L3);
  
  return cach;
}

struct frequency* get_frequency_info() {
  struct frequency* freq = malloc(sizeof(struct frequency));
  
  freq->max = get_freq_from_file(_PATH_FREQUENCY_MAX);
  freq->min = get_freq_from_file(_PATH_FREQUENCY_MIN);
  
  return freq;
}

void free_cache_struct(struct cache* cach) {
  free(cach);
}

void free_freq_struct(struct frequency* freq) {
  free(freq);
}

void debugCache(struct cache* cach) {
  printf("L1i=%dB\n",cach->L1i);
  printf("L1d=%dB\n",cach->L1d);
  printf("L2=%dB\n",cach->L2);
  printf("L3=%dB\n",cach->L3);
}

void debugFrequency(struct frequency* freq) {
  printf("max f=%ldMhz\n",freq->max);
  printf("min f=%ldMhz\n",freq->min);
}
