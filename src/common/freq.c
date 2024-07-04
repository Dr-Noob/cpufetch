#ifdef __linux__

#define _GNU_SOURCE

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>

#include "global.h"

static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags) {
    int ret;
    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                    group_fd, flags);
    return ret;
}

#define INSERT_ASM_ONCE __asm volatile("nop");
#define INSERT_ASM_10_TIMES \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \
    INSERT_ASM_ONCE         \

#define INSERT_ASM_100_TIMES \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES      \
    INSERT_ASM_10_TIMES

#define INSERT_ASM_1000_TIMES \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \
    INSERT_ASM_100_TIMES     \

void nop_function(uint64_t iters) {
  for (uint64_t i = 0; i < iters; i++) {
    INSERT_ASM_1000_TIMES
    INSERT_ASM_1000_TIMES
    INSERT_ASM_1000_TIMES
    INSERT_ASM_1000_TIMES
  }
}

// Differences between x86 measure_frequency this measure_max_frequency:
// - measure_frequency employs all cores simultaneously wherease 
//   measure_max_frequency only employs 1.
// - measure_frequency runs the computation and checks /proc/cpuinfo whereas
//   measure_max_frequency does not rely on /proc/cpuinfo and simply
//   counts cpu cycles to measure frequency.
// - measure_frequency uses actual computation while measuring the frequency
//   whereas measure_max_frequency uses nop instructions. This makes the former
//   x86 dependant whereas the latter is architecture independant.
int64_t measure_max_frequency(uint32_t core) {
  if (!bind_to_cpu(core)) {
    printErr("Failed binding the process to CPU %d", core);
    return -1;
  }

  clockid_t clock = CLOCK_PROCESS_CPUTIME_ID;

  struct perf_event_attr pe;
  uint64_t instructions;
  int fd;
  int pid = 0;

  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = PERF_COUNT_HW_CPU_CYCLES;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;

  fd = perf_event_open(&pe, pid, core, -1, 0);
  if (fd == -1) {
    perror("perf_event_open");
    if (errno == EPERM || errno == EACCES) {
      printf("You may not have permission to collect stats.\n");
      printf("Consider tweaking /proc/sys/kernel/perf_event_paranoid or running as root.\n");
    }
    return -1;
  }

  const char* frequency_banner = "cpufetch is measuring the max frequency...";
  printf("%s", frequency_banner);
  fflush(stdout);

  uint64_t iters = 10000000;
  struct timespec start, end;
  if (clock_gettime(clock, &start) == -1) {
    perror("clock_gettime");
    return -1;
  }
  if(ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1) {
    perror("ioctl");
    return -1;
  }
  if(ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1) {
    perror("ioctl");
    return -1;
  }

  nop_function(iters);

  read(fd, &instructions, sizeof(uint64_t));
  if(ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) == -1) {
    perror("ioctl");
    return -1;
  }
  if (clock_gettime(clock, &end) == -1) {
    perror("clock_gettime");
    return -1;
  }

  uint64_t nsecs = (end.tv_sec*1e9 + end.tv_nsec) - (start.tv_sec*1e9 + start.tv_nsec);
  uint64_t usecs = nsecs/1000;  
  double frequency = instructions/((double)usecs);

  printf("\r%*c\r", (int) strlen(frequency_banner), ' ');
  
  // Discard last digit in the frequency, which should help providing
  // more reliable and predictable values.
  return (((int) frequency + 5)/10) * 10;
}

#endif // #ifdef __linux__
