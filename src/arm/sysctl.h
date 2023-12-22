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
#define MIDR_APPLE_M2_SAWTOOTH  0x610F0480
#define MIDR_APPLE_M2_EVEREST   0x610F0490

// M1 / A14
#ifndef CPUFAMILY_ARM_FIRESTORM_ICESTORM
  #define CPUFAMILY_ARM_FIRESTORM_ICESTORM 0x1B588BB3
#endif
// M2 / A15
#ifndef CPUFAMILY_ARM_AVALANCHE_BLIZZARD
  #define CPUFAMILY_ARM_AVALANCHE_BLIZZARD 0xDA33D83D
#endif
// M3 / A16 / A17
#ifndef CPUFAMILY_ARM_EVEREST_SAWTOOTH
  #define CPUFAMILY_ARM_EVEREST_SAWTOOTH 0x72015832
#endif

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

uint32_t get_sys_info_by_name(char* name);

#endif
