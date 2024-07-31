#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#ifdef __linux__
  #include <sys/auxv.h>
  #include <asm/hwcap.h>
  #include "../common/freq.h"
#elif defined __APPLE__ || __MACH__
  #include "../common/sysctl.h"
#endif

#include "../common/global.h"
#include "../common/soc.h"
#include "../common/args.h"
#include "udev.h"
#include "midr.h"
#include "uarch.h"

// https://docs.kernel.org/arch/arm64/elf_hwcaps.html
void check_for_SVE(void) {
  errno = 0;
  long hwcaps = getauxval(AT_HWCAP);

  if(errno == ENOENT) {
    printWarn("Unable to retrieve AT_HWCAP using getauxval");
  }

  if (hwcaps & HWCAP_SVE) printf("SVE: Yes\n");
  else printf("SVE: No\n");

  hwcaps = getauxval(AT_HWCAP2);

  if (hwcaps & HWCAP2_SVE2) printf("SVE2: Yes\n");
  else printf("SVE2: No\n");
}

#include <arm_sve.h>

#ifndef __ARM_FEATURE_SVE
#warning "Make sure to compile for SVE!"
#endif

// https://learn.arm.com/learning-paths/servers-and-cloud-computing/sve/sve_basics/#:~:text=Using%20a%20text%20editor%20of%20your%20choice%2C%20copy,svcntb%28%29%29%3B%20%7D%20This%20program%20prints%20the%20vector%20length
void SVE_exp(struct cpuInfo* cpu) {
  check_for_SVE();

  uint32_t core = 4;
  if (!bind_to_cpu(core)) {
    printErr("Failed binding the process to CPU %d", core);
    return;
  }
  printf("[%d] SVE vector length is: %ld bytes\n", core, svcntb());

  core = 7;
  if (!bind_to_cpu(core)) {
    printErr("Failed binding the process to CPU %d", core);
    return;
  }
  printf("[%d] SVE vector length is: %ld bytes\n", core, svcntb());

  core = 0;
  if (!bind_to_cpu(core)) {
    printErr("Failed binding the process to CPU %d", core);
    return;
  }
  printf("[%d] SVE vector length is: %ld bytes\n", core, svcntb());
}
