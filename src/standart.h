#ifndef __01h__
#define __01h__

#define VENDOR_EMPTY   0
#define VENDOR_INTEL   1
#define VENDOR_AMD     2
#define VENDOR_INVALID 3

typedef int VENDOR;

struct cpuInfo* get_cpu_info();
VENDOR get_cpu_vendor(struct cpuInfo* cpu);

char* get_str_peak_performance(struct cpuInfo* cpu, long freq);
char* get_str_ncores(struct cpuInfo* cpu);
char* get_str_avx(struct cpuInfo* cpu);
char* get_str_sse(struct cpuInfo* cpu);
char* get_str_fma(struct cpuInfo* cpu);
char* get_str_aes(struct cpuInfo* cpu);
char* get_str_sha(struct cpuInfo* cpu);

void debug_cpu_info(struct cpuInfo* cpu);

#endif
