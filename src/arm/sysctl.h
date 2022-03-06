#ifndef __SYSCTL__
#define __SYSCTL__

// From Linux kernel: arch/arm64/include/asm/cputype.h
#define MIDR_APPLE_M1_ICESTORM  0x610F0220
#define MIDR_APPLE_M1_FIRESTORM 0x610F0230
#ifndef CPUFAMILY_ARM_FIRESTORM_ICESTORM
  #define CPUFAMILY_ARM_FIRESTORM_ICESTORM 0x1B588BB3
#endif
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
