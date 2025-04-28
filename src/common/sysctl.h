#ifndef __SYSCTL__
#define __SYSCTL__

// From Linux kernel: arch/arm64/include/asm/cputype.h
#define MIDR_APPLE_M1_ICESTORM  0x610F0220
#define MIDR_APPLE_M1_FIRESTORM 0x610F0230
// Kernel does not include those, so I just assume that
// APPLE_CPU_PART_M2_BLIZZARD=0x30,M2_AVALANCHE=0x31
#define MIDR_APPLE_M2_BLIZZARD  0x610F0300
#define MIDR_APPLE_M2_AVALANCHE 0x610F0310
// https://github.com/AsahiLinux/m1n1/blob/main/src/chickens.c
#define MIDR_APPLE_M3_SAWTOOTH  0x610F0480
#define MIDR_APPLE_M3_EVEREST   0x610F0490

// M1 / A14
#ifndef CPUFAMILY_ARM_FIRESTORM_ICESTORM
  #define CPUFAMILY_ARM_FIRESTORM_ICESTORM 0x1B588BB3
#endif
// M2 / A15
#ifndef CPUFAMILY_ARM_AVALANCHE_BLIZZARD
  #define CPUFAMILY_ARM_AVALANCHE_BLIZZARD 0xDA33D83D
#endif
// M3 / A16 / A17
// M3:   https://ratfactor.com/zig/stdlib-browseable2/c/darwin.zig.html
// M3_2: https://github.com/Dr-Noob/cpufetch/issues/230
// PRO:  https://github.com/Dr-Noob/cpufetch/issues/225
// MAX:  https://github.com/Dr-Noob/cpufetch/issues/210
#define CPUFAMILY_ARM_EVEREST_SAWTOOTH     0x8765EDEA
#define CPUFAMILY_ARM_EVEREST_SAWTOOTH_2   0xFA33415E
#define CPUFAMILY_ARM_EVEREST_SAWTOOTH_PRO 0x5F4DEA93
#define CPUFAMILY_ARM_EVEREST_SAWTOOTH_MAX 0x72015832
#define CPUFAMILY_ARM_M4_TEMP 0x6F5129AC
// For detecting different M1 types
// NOTE: Could also be achieved detecting different
// MIDR values (e.g., APPLE_CPU_PART_M1_ICESTORM_PRO)
#ifndef CPUSUBFAMILY_ARM_HG
  #define CPUSUBFAMILY_ARM_HG 2
#endif
#ifndef CPUSUBFAMILY_ARM_HS
  #define CPUSUBFAMILY_ARM_HS 4
#endif
#ifndef CPUSUBFAMILY_ARM_HC_HD
  #define CPUSUBFAMILY_ARM_HC_HD 5
#endif

// For alternative way to get CPU frequency on macOS and *BSD
#ifdef __APPLE__
  #define CPUFREQUENCY_SYSCTL "hw.cpufrequency_max"
#else
  // For FreeBSD, not sure about other *BSD
  #define CPUFREQUENCY_SYSCTL "dev.cpu.0.freq"
#endif

uint32_t get_sys_info_by_name(char* name);

#endif
