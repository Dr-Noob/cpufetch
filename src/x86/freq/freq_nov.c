#include <stdio.h>
#include <immintrin.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include "freq.h"

void* compute_nov() {
  bool end = false;

  struct timeval begin, now;

  float a = 1.5;
  float b = 1.2;
  float c = 0.0;

  gettimeofday(&begin, NULL);
  while(!end) {
    for(uint64_t i=0; i < LOOP_ITERS; i++) {
      c = a * b;
    }

    gettimeofday(&now, NULL);
    double elapsed = (now.tv_sec - begin.tv_sec) + ((now.tv_usec - begin.tv_usec)/1000000.0);
    end = elapsed >= (double) MEASURE_TIME_SECONDS;
  }

  FILE* fp = fopen("/dev/null", "w");
  if(fp == NULL) {
    printf("fopen: %s", strerror(errno));
  }
  else {
    fprintf(fp, "%f", c);
    fclose(fp);
  }

  return NULL;
}

