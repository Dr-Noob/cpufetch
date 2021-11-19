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

void* compute_avx512() {
  bool end = false;

  struct timeval begin, now;

  __m512 a = _mm512_set1_ps(1.5);
  __m512 b = _mm512_set1_ps(1.2);
  __m512 c = _mm512_set1_ps(0.0);

  gettimeofday(&begin, NULL);
  while(!end) {
    for(uint64_t i=0; i < LOOP_ITERS; i++) {
      c = _mm512_fmadd_ps(a, b, c);
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
    fprintf(fp, "%f", c[0]);
    fclose(fp);
  }

  return NULL;
}

