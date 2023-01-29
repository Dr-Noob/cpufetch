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

void* compute_avx(void * pthread_arg) {
  UNUSED(pthread_arg);
  bool end = false;

  struct timeval begin, now;

  __m256 a[8];
  __m256 b[8];

  for(int i=0; i < 8; i++) {
    a[i] = _mm256_set1_ps(1.5);
    b[i] = _mm256_set1_ps(1.2);
  }

  gettimeofday(&begin, NULL);
  while(!end) {
    for(uint64_t i=0; i < LOOP_ITERS; i++) {
      a[0] = _mm256_add_ps(a[0], b[0]);
      a[1] = _mm256_add_ps(a[1], b[1]);
      a[2] = _mm256_add_ps(a[2], b[2]);
      a[3] = _mm256_add_ps(a[3], b[3]);
      a[4] = _mm256_add_ps(a[4], b[4]);
      a[5] = _mm256_add_ps(a[5], b[5]);
      a[6] = _mm256_add_ps(a[6], b[6]);
      a[7] = _mm256_add_ps(a[7], b[7]);
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
    for(int i=0; i < 8; i++)
      fprintf(fp, "%f", a[i][0]);
    fclose(fp);
  }

  return NULL;
}

