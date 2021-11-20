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

void* compute_avx() {
  bool end = false;

  struct timeval begin, now;

  __m256 a = _mm256_set1_ps(1.5);
  __m256 b = _mm256_set1_ps(1.2);

  gettimeofday(&begin, NULL);
  while(!end) {
    for(uint64_t i=0; i < LOOP_ITERS; i++) {
      a = _mm256_add_ps(a, b);
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
    fprintf(fp, "%f", a[0]);
    fclose(fp);
  }

  return NULL;
}

