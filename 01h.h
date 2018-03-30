#ifndef __01h__
#define __01h__

#define STRING_YES  "Yes"
#define STRING_NO   "No"

#define VENDOR_EMPTY   0
#define VENDOR_INTEL   1
#define VENDOR_AMD     2
#define VENDOR_INVALID 3
#define VENDOR_INTEL_STRING "GenuineIntel"
#define VENDOR_AMD_STRING   "AuthenticAMD"
typedef int VENDOR;

struct cpuInfo;

struct cpuInfo* getCPUInfo();
void debugCpuInfo(struct cpuInfo* cpu);

VENDOR getCPUVendorInternal(struct cpuInfo* cpu);
char* getPeakPerformance(struct cpuInfo* cpu, long freq);

char* getString_NumberCores(struct cpuInfo* cpu);
char* getString_AVX(struct cpuInfo* cpu);
char* getString_SSE(struct cpuInfo* cpu);
char* getString_FMA(struct cpuInfo* cpu);
char* getString_AES(struct cpuInfo* cpu);
char* getString_SHA(struct cpuInfo* cpu);

#endif
