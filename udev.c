#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "udev.h"

struct cache {
  int L1i;
  int L1d;
  int L2;
  int L3;
};

struct frequency {
  int max;
  int min;
};

/***

Parses buf which should be expressed in the way:
xxxxK where 'x' are numbers and 'K' refers to kilobytes.
Returns the size as a int in bytes

***/

int getSize(char* buf, int size) {
  char* end = strstr (buf,"K");
  if(end == NULL) {
    printf("ERROR in getSize(strstr)\n");
    return NO_CACHE;
  }
  *end = 0;
  int cachsize = atoi(buf);
  if(cachsize == 0) {
    printf("ERROR in getSize(atoi)\n");
    return NO_CACHE;
  }
  return cachsize;
}

/***

Returns size(in bytes) of cache described by path or
NO_CACHE if the cache doest no exists

***/

//Sacar factor comun lectura
//Block pasar de 1 a 1000
int getCache(char* path) {
  FILE *file = fopen(path, "r");

	if(file == NULL) {
    //Doest not exist
		return NO_CACHE;
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

  int ret = getSize(buf,offset)*1024;
	free(buf);
  return ret;
}

int getFrequency(char* path) {
  FILE *file = fopen(path, "r");

  if(file == NULL) {
    //Doest not exist
    return NO_CACHE;
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
    printf("error in getFrequency\n");
    return NO_CACHE;
  }
  return ret/1000;
}

/*** GET_STRING ***/

char* getString_L1(struct cache* cach) {
  //Max 2 digits,2 for 'KB',6 for '(Data)'
  //and 14 for '(Instructions)'
  int size = (2*(2+2)+6+14+1);
  char* string = malloc(sizeof(char)*size);
  snprintf(string,size,"%dKB(Data)%dKB(Instructions)",cach->L1d/1024,cach->L1i/1024);
  return string;
}

char* getString_L2(struct cache* cach) {
  if(cach->L2 == NO_CACHE) {
    char* string = malloc(sizeof(char)*5);
    snprintf(string,5,"None");
    return string;
  }
  else {
    //Max 4 digits and 2 for 'KB'
    int size = (4+2+1);
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%dKB",cach->L2/1024);
    return string;
  }
}

char* getString_L3(struct cache* cach) {
  if(cach->L3 == NO_CACHE) {
    char* string = malloc(sizeof(char)*5);
    snprintf(string,5,"None");
    return string;
  }
  else {
    //Max 4 digits and 2 for 'KB'
    int size = (4+2+1);
    char* string = malloc(sizeof(char)*size);
    snprintf(string,size,"%dKB",cach->L3/1024);
    return string;
  }
}

char* getString_MaxFrequency(struct frequency* freq) {
  //Max 4 digits and 3 for 'MHz' plus 1 for '\0'
  int size = (4+3+1);
  char* string = malloc(sizeof(char)*size);
  snprintf(string,size,"%dMHz",freq->max);
  return string;
}

/*** CREATE DEBUGING AND FREES ***/

struct cache* new_cache(struct cache* cach) {
  cach = malloc(sizeof(struct cache));
  cach->L1i = getCache(_PATH_CACHE_L1i);
  cach->L1d = getCache(_PATH_CACHE_L1d);
  cach->L2 = getCache(_PATH_CACHE_L2);
  cach->L3 = getCache(_PATH_CACHE_L3);
  return cach;
}

struct frequency* new_frequency(struct frequency* freq) {
  freq = malloc(sizeof(struct frequency));
  freq->max = getFrequency(_PATH_FREQUENCY_MAX);
  freq->min = getFrequency(_PATH_FREQUENCY_MIN);
  return freq;
}

void debugCache(struct cache* cach) {
  printf("L1i=%dB\n",cach->L1i);
  printf("L1d=%dB\n",cach->L1d);
  printf("L2=%dB\n",cach->L2);
  printf("L3=%dB\n",cach->L3);
}

void debugFrequency(struct frequency* freq) {
  printf("max f=%dMhz\n",freq->max);
  printf("min f=%dMhz\n",freq->min);
}

void freeCache(struct cache* cach) {
  free(cach);
}

void freeFrequency(struct frequency* freq) {
  free(freq);
}
