#include <stdint.h>

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

void nop_function_arm(uint64_t iters) {
  for (uint64_t i = 0; i < iters; i++) {
    INSERT_ASM_1000_TIMES
    INSERT_ASM_1000_TIMES
    INSERT_ASM_1000_TIMES
    INSERT_ASM_1000_TIMES
  }
}
