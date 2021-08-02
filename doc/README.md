# cpufetch programming documentation (v0.98)
This documentation explains how cpufetch works internally and all the design decisions I made. This document intends to be useful for me in the future, for everyone interested in the project, and for anyone who is trying to obtain any specific information from the CPU. In this way, this can be used as a manual or a page that collects interesting material in this area.

### 1. Basics
cpufetch works for __x86_64__ (Intel and AMD),  __ARM__ and __PowerPC__ CPUs. However, cpufetch is expected to work better on x86_64, because the codebase is older and has been tested much more than the ARM and PowerPC versions. Depending on the architecture, cpufetch choose certain files to be compiled. A summarized tree of the source code of cpufetch is shown below.

```
cpufetch/
├── doc
│   ├── DOCUMENTATION_ARM.md
|   ├── DOCUMENTATION_PPC.md
│   ├── DOCUMENTATION_X86.md
│   └── README.md
├── Makefile
├── README.md
└── src/
    ├── arm/
    │   ├── midr.c
    │   ├── midr.h
    │   └── other files ...
    ├── common/
    │   └── common files ...
    ├── ppc/
    |   ├── ppc.c
    |   ├── ppc.h
    |   └── other files ...
    └── x86/
        ├── cpuid.c
        ├── cpuid.h
        └── other files ...
```

Source code is divided into four directories:

- `common/`: Source code shared between all architectures
- `arm/`: ARM source code
- `ppc/`: PowerPC source code
- `x86/`: x86 source code

##### 1.1 Basics (x86_64)

In x86, __cpufetch works using the CPUID instruction__. It is called directly using assembly (see `src/x86/cpuid_asm.c`). To understand how CPUID works, see [DOCUMENTATION_X86.md](https://github.com/Dr-Noob/cpufetch/blob/master/doc/DOCUMENTATION_X86.md).

At the beginning of execution, cpufetch needs to know the max standard CPUID level and max CPUID extended level supported in the running CPU. We also need to know if the x86 CPU is Intel or AMD because sometimes, the way to obtain the information depends on the manufacturer. This information will be stored in:

```
struct cpuInfo {
  ...
  VENDOR cpu_vendor;
  uint32_t maxLevels;  
  uint32_t maxExtendedLevels;
  ...
};
```

To use any CPUID leaf, cpufetch always needs to check that it is supported in the current CPU.

##### 1.2 Basics (ARM)
In ARM, __cpufetch works using the MIDR register and Linux filesystem__. MIDR (Main ID Register) is read from `/proc/cpuinfo`. It allows the detection of the microarchitecture of the cores. Furthermore, Linux filesystem `/sys/devices/system/cpu/` is used to fetch the number of cores and other information. This is the main reason to explain __why `cpufetch` for ARM only works on Linux systems.__

##### 1.3 Basics (PowerPC)
In PowerPC, __cpufetch works using the PVR register and Linux filesystem__. PVR (Processor Version Register) is read using assembly and it is used to identify the microarchitecture of the CPU. Linux is also used to query the rest of the information, like the CPU topology, frequency, etc. This is the main reason to explain __why `cpufetch` for PowerPC only works on Linux systems.__

##### 1.4 Documentation organization
The rest of the documentation is divided in specific files for each architecture, since each one needs different implementations:

- [DOCUMENTATION_ARM.md](https://github.com/Dr-Noob/cpufetch/blob/master/doc/DOCUMENTATION_ARM.md)
- [DOCUMENTATION_PPC.md](https://github.com/Dr-Noob/cpufetch/blob/master/doc/DOCUMENTATION_PPC.md)
- [DOCUMENTATION_X86.md](https://github.com/Dr-Noob/cpufetch/blob/master/doc/DOCUMENTATION_X86.md)
