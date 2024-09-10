#define  _GNU_SOURCE
#include <stdio.h>

#include "../../common/global.h"
#include "../uarch.h"
#include "freq.h"
#include "freq_nov.h"
#include "freq_avx.h"
#include "freq_avx512.h"

#include <immintrin.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define MAX_NUMBER_THREADS         512
#define FREQ_VECTOR_SIZE         1<<16

struct freq_thread {
  // Inputs
  struct cpuInfo* cpu;
  bool end;
  bool measure;
  // Output
  int32_t *max_pp;
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
  struct cpuInfo* cpu = freq->cpu;

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

  if (cpu->hybrid_flag) {
    // We have an heterogeneous architecture. After measuring the
    // frequency for all cores, we now need to compute the average
    // independently for each CPU module.
    struct cpuInfo* ptr = cpu;
    double* freq_vector_ptr = freq_vector;

    for (int i=0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
      freq->max_pp[i] = vector_average_harmonic(freq_vector_ptr, ptr->topo->total_cores_module);
      printWarn("AVX2 measured freq=%d (module %d)", freq->max_pp[i], i);

      freq_vector_ptr = freq_vector_ptr + ptr->topo->total_cores_module;
    }
  }
  else {
    freq->max_pp[0] = vector_average_harmonic(freq_vector, v);
    printWarn("AVX2 measured freq=%d\n", freq->max_pp[0]);
  }

  return NULL;
}

int32_t measure_frequency(struct cpuInfo* cpu, int32_t *max_freq_pp_vec) {
  if (cpu->hybrid_flag && cpu->module_id > 0) {
    // We have a hybrid architecture and we have already
    // measured the frequency for this module in a previous
    // call to this function, so now just return it.
    return max_freq_pp_vec[cpu->module_id];
  }

  int ret;
  int num_spaces;
  struct freq_thread* freq_struct = malloc(sizeof(struct freq_thread));
  freq_struct->end = false;
  freq_struct->measure = false;
  freq_struct->cpu = cpu;
  freq_struct->max_pp = max_freq_pp_vec;

  void* (*compute_function)(void*);

  if(cpu->feat->AVX512 && vpus_are_AVX512(cpu)) {
    printf("cpufetch is measuring the AVX512 frequency...");
    compute_function = compute_avx512;
    num_spaces = 45;
  }
  else if(cpu->feat->AVX || cpu->feat->AVX2) {
    printf("cpufetch is measuring the AVX frequency...");
    compute_function = compute_avx;
    num_spaces = 42;
  }
  else {
    printf("cpufetch is measuring the frequency (no vector instructions)...");
    compute_function = compute_nov;
    num_spaces = 63;
  }

  fflush(stdout);

  pthread_t freq_t;
  if(pthread_create(&freq_t, NULL, measure_freq, freq_struct)) {
    fprintf(stderr, "Error creating thread\n");
    return -1;
  }

  pthread_t* compute_th = malloc(sizeof(pthread_t) * cpu->topo->total_cores);
  cpu_set_t cpus;
  pthread_attr_t attr;
  if ((ret = pthread_attr_init(&attr)) != 0) {
    printErr("pthread_attr_init: %s", strerror(ret));
    return -1;
  }

  for(int i=0; i < cpu->topo->total_cores; i++) {
    // We might have called bind_to_cpu previously, binding the threads
    // to a specific core, so now we must make sure we run the new thread
    // on the correct core.
    CPU_ZERO(&cpus);
    CPU_SET(i, &cpus);
    if ((ret = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus)) != 0) {
      printErr("pthread_attr_setaffinity_np: %s", strerror(ret));
      return -1;
    }

    ret = pthread_create(&compute_th[i], &attr, compute_function, NULL);

    if(ret != 0) {
      fprintf(stderr, "Error creating thread\n");
      return -1;
    }
  }

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

  printf("\r%*c", num_spaces, ' ');
  return max_freq_pp_vec[cpu->module_id];
}
