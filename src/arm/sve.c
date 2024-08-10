#include <stdint.h>
#include "../common/global.h"

// https://learn.arm.com/learning-paths/servers-and-cloud-computing/sve/sve_basics/#:~:text=Using%20a%20text%20editor%20of%20your%20choice%2C%20copy,svcntb%28%29%29%3B%20%7D%20This%20program%20prints%20the%20vector%20length
uint64_t sve_cntb(void) {
  #ifdef __ARM_FEATURE_SVE
    uint64_t x0 = 0;
    __asm volatile("cntb %0"
         : "=r"(x0));
    return x0;
  #else
    printWarn("sve_cntb: Hardware supports SVE, but it was not enabled by the compiler");
    return 0;
  #endif
}
