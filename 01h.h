#ifndef __01h__
#define __01h__

struct cpuInfo;

struct cpuInfo* getCPUInfo();
void debugCpuInfo(struct cpuInfo* cpu);

char* getString_NumberCores(struct cpuInfo* cpu);
char* getString_AVX(struct cpuInfo* cpu);
char* getString_SSE(struct cpuInfo* cpu);
char* getString_FMA(struct cpuInfo* cpu);
char* getString_AES(struct cpuInfo* cpu);
char* getString_SHA(struct cpuInfo* cpu);

#endif
