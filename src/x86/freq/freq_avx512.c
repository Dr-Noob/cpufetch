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

/*
 * For AVX512, it seems that multiple independent
 * instructions are needed to force the CPU to
 * use AVX512 frequency, since with only one instruction
 * (as the AVX implementaion) it still uses AVX frequency
 */
void* compute_avx512() {
  bool end = false;

  struct timeval begin, now;

  __m512 a[8];
  __m512 b[8];
  __m512 mult;

  for(int i=0; i < 8; i++) {
    a[i] = _mm512_set1_ps(1.5);
    b[i] = _mm512_set1_ps(1.2);
  }

  gettimeofday(&begin, NULL);
  while(!end) {
    for(uint64_t i=0; i < LOOP_ITERS; i++) {
      a[0] = _mm512_fmadd_ps(mult, a[0], b[0]);
      a[1] = _mm512_fmadd_ps(mult, a[1], b[1]);
      a[2] = _mm512_fmadd_ps(mult, a[2], b[2]);
      a[3] = _mm512_fmadd_ps(mult, a[3], b[3]);
      a[4] = _mm512_fmadd_ps(mult, a[4], b[4]);
      a[5] = _mm512_fmadd_ps(mult, a[5], b[5]);
      a[6] = _mm512_fmadd_ps(mult, a[6], b[6]);
      a[7] = _mm512_fmadd_ps(mult, a[7], b[7]);
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

