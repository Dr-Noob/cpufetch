#define  _GNU_SOURCE
#include <stdio.h>

#include "freq.h"
#include "../common/global.h"

#include <immintrin.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define MEASURE_TIME_SECONDS         5
#define MAX_NUMBER_THREADS         512
#define FREQ_VECTOR_SIZE         1<<16
#define LOOP_ITERS           100000000

struct freq_thread {
  bool end;
  bool measure;
  double freq;
};

double vector_average_harmonic(double* v, int len) {
  double acc = 0.0;
  for(int i=0; i < len; i++) {
    acc += 1 / v[i];
  }
  return len / acc;
}

void sleep_ms(int64_t ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, &ts);
}

void* measure_freq(void *freq_ptr) {
  struct freq_thread* freq = (struct freq_thread*) freq_ptr;

  char* end = NULL;
  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  int v = 0;
  double* freq_vector = malloc(sizeof(double) * FREQ_VECTOR_SIZE);

  while(!freq->end) {
    if(!freq->measure) continue;

    FILE* fp = fopen("/proc/cpuinfo", "r");
    if(fp == NULL) return NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
      if((line = strstr(line, "cpu MHz")) != NULL) {
        line = strstr(line, "\t: ");
        if(line == NULL) return NULL;
        line += sizeof("\t: ") - 1;
        double f = strtold(line, &end);
        if(errno != 0) {
          printf("strtol: %s", strerror(errno));
          return NULL;
        }

        freq_vector[v] = f;
        v++;
      }
    }
    fclose(fp);
    sleep_ms(500);
  }

  freq->freq = vector_average_harmonic(freq_vector, v);
  printWarn("AVX2 measured freq=%f\n", freq->freq);

  return NULL;
}

void* compute_avx() {
  bool end = false;

  struct timeval begin, now;

  __m256 a = _mm256_set1_ps(1.5);
  __m256 b = _mm256_set1_ps(1.2);
  __m256 c = _mm256_set1_ps(0.0);

  gettimeofday(&begin, NULL);
  while(!end) {
    for(uint64_t i=0; i < LOOP_ITERS; i++) {
      c = _mm256_fmadd_ps(a, b, c);
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

int64_t measure_avx_frequency(struct cpuInfo* cpu) {
  int ret;
  struct freq_thread* freq_struct = malloc(sizeof(struct freq_thread));
  freq_struct->end = false;
  freq_struct->measure = false;

  pthread_t freq_t;
  if(pthread_create(&freq_t, NULL, measure_freq, freq_struct)) {
    fprintf(stderr, "Error creating thread\n");
    return -1;
  }

  pthread_t* compute_th = malloc(sizeof(pthread_t) * cpu->topo->total_cores);
  for(int i=0; i < cpu->topo->total_cores; i++) {
    ret = pthread_create(&compute_th[i], NULL, compute_avx, NULL);

    if(ret != 0) {
      fprintf(stderr, "Error creating thread\n");
      return -1;
    }
  }

  printf("cpufetch is measuring AVX2 frequency...");
  fflush(stdout);

  sleep_ms(500);
  freq_struct->measure = true;

  for(int i=0; i < cpu->topo->total_cores; i++) {
    if(pthread_join(compute_th[i], NULL)) {
      fprintf(stderr, "Error joining thread\n");
      return -1;
    }
    freq_struct->end = true;
  }

  if(pthread_join(freq_t, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return -1;
  }

  printf("\r                                       ");
  return freq_struct->freq;
}
