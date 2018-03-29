#ifndef __UDEV__
#define __UDEV__

/*** PATHS ***/

#define _PATH_SYS_SYSTEM	"/sys/devices/system"
#define _PATH_SYS_CPU		_PATH_SYS_SYSTEM"/cpu"
#define _PATH_ONE_CPU   _PATH_SYS_CPU"/cpu0"

#define _PATH_FREQUENCY _PATH_ONE_CPU"/cpufreq"
#define _PATH_FREQUENCY_MAX _PATH_FREQUENCY"/cpuinfo_max_freq"
#define _PATH_FREQUENCY_MIN _PATH_FREQUENCY"/cpuinfo_min_freq"

#define _PATH_CPU_CACHE _PATH_ONE_CPU"/cache"
#define _PATH_CACHE_L1d _PATH_CPU_CACHE"/index0/size"
#define _PATH_CACHE_L1i _PATH_CPU_CACHE"/index1/size"
#define _PATH_CACHE_L2  _PATH_CPU_CACHE"/index2/size"
#define _PATH_CACHE_L3  _PATH_CPU_CACHE"/index3/size"

/*** CONSTANTS ***/

#define NO_CACHE -1
#define DEFAULT_FILE_SIZE 4096

/*** STRUCTS ***/

struct cache;
struct frequency;

/*** FUNCTIONS ***/

struct cache* new_cache(struct cache* cach);
void debugCache(struct cache* cach);
void freeCache(struct cache* cach);
char* getString_L1(struct cache* cach);
char* getString_L2(struct cache* cach);
char* getString_L3(struct cache* cach);

struct frequency* new_frequency(struct frequency* freq);
void debugFrequency(struct frequency* freq);
void freeFrequency(struct frequency* freq);
char* getString_MaxFrequency(struct frequency* freq);

#endif
