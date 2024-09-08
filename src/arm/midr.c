#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#ifdef __linux__
  #include <sys/auxv.h>
  #include <asm/hwcap.h>
  #include "../common/freq.h"
#elif defined __APPLE__ || __MACH__
  #include "../common/sysctl.h"
#elif defined _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
#endif

#include "../common/global.h"
#include "../common/soc.h"
#include "../common/args.h"
#include "udev.h"
#include "midr.h"
#include "uarch.h"
#include "sve.h"


#if defined _WIN32
// Windows stores processor information in registery at:
// "HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor"
// Within this directory, each core will get its own folder with
// registery entries named `CP ####` that map to ARM system registers.
// Ex. the MIDR register for core 0 is the `REG_QWORD` at:
// "HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0\CP 4000"
// The name of these `CP ####`-registers follow their register ID encoding in hexadecimal
// (op0&1):op1:crn:crm:op2.
// More registers can be found here:
// https://developer.arm.com/documentation/ddi0601/2024-06/AArch64-Registers
// Some important ones:
// CP 4000: MIDR_EL1
// CP 4020: ID_AA64PFR0_EL1
// CP 4021: ID_AA64PFR1_EL1
// CP 4028: ID_AA64DFR0_EL1
// CP 4029: ID_AA64DFR1_EL1
// CP 402C: ID_AA64AFR0_EL1
// CP 402D: ID_AA64AFR1_EL1
// CP 4030: ID_AA64ISAR0_EL1
// CP 4031: ID_AA64ISAR1_EL1
// CP 4038: ID_AA64MMFR0_EL1
// CP 4039: ID_AA64MMFR1_EL1
// CP 403A: ID_AA64MMFR2_EL1

bool read_registry_hklm_int(char* path, char* name, void* value, bool is64) {  
  DWORD value_len;
  int reg_type;
  if (is64) {
    value_len = sizeof(int64_t);
    reg_type = RRF_RT_REG_QWORD;
  }
  else {
    value_len = sizeof(int32_t);
    reg_type = RRF_RT_REG_DWORD;
  }

  if(RegGetValueA(HKEY_LOCAL_MACHINE, path, name, reg_type, NULL, value, &value_len) != ERROR_SUCCESS) {
    printBug("Error reading registry entry \"%s\\%s\"", path, name);
    return false;
  }
  return true;
}

bool get_win32_core_info_int(uint32_t core_index, char* name, void* value, bool is64) {
  // path + digits
  uint32_t max_path_size = 45+3+1;
  char* path = ecalloc(sizeof(char) * max_path_size, sizeof(char));
  snprintf(path, max_path_size, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%u", core_index);
  return read_registry_hklm_int(path, name, value, is64);
}
#endif

bool cores_are_equal(int c1pos, int c2pos, uint32_t* midr_array, int32_t* freq_array) {
  return midr_array[c1pos] == midr_array[c2pos] && freq_array[c1pos] == freq_array[c2pos];
}

struct cache* get_cache_info(struct cpuInfo* cpu) {
  struct cache* cach = emalloc(sizeof(struct cache));
  init_cache_struct(cach);

  cach->max_cache_level = 2;
  for(int i=0; i < cach->max_cache_level + 1; i++) {
    cach->cach_arr[i]->exists = true;
    cach->cach_arr[i]->num_caches = 1;
    cach->cach_arr[i]->size = 0;
  }

  return cach;
}

struct frequency* get_frequency_info(uint32_t core) {
  struct frequency* freq = emalloc(sizeof(struct frequency));

  freq->measured = false;
  freq->base = UNKNOWN_DATA;
  freq->max = get_max_freq_from_file(core);
  #ifdef __linux__
    if (freq->max == UNKNOWN_DATA || measure_max_frequency_flag()) {
      if (freq->max == UNKNOWN_DATA)
        printWarn("Unable to find max frequency from udev, measuring CPU frequency");
      freq->max = measure_max_frequency(core);
      freq->measured = true;
    }
  #endif

  return freq;
}

struct topology* get_topology_info(struct cpuInfo* cpu, struct cache* cach, uint32_t* midr_array, int32_t* freq_array, int socket_idx, int ncores) {
  struct topology* topo = emalloc(sizeof(struct topology));
  init_topology_struct(topo, cach);

  int sockets_seen = 0;
  int first_core_idx = 0;
  int currrent_core_idx = 0;
  int cores_in_socket = 0;

  while(socket_idx + 1 > sockets_seen) {
    if(currrent_core_idx < ncores && cores_are_equal(first_core_idx, currrent_core_idx, midr_array, freq_array)) {
      currrent_core_idx++;
      cores_in_socket++;
    }
    else {
      topo->total_cores = cores_in_socket;
      cores_in_socket = 0;
      first_core_idx = currrent_core_idx;
      sockets_seen++;
    }
  }

  return topo;
}

int64_t get_peak_performance(struct cpuInfo* cpu) {
  struct cpuInfo* ptr = cpu;

  //First check we have consistent data
  for(int i=0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
    if(get_freq(ptr->freq) == UNKNOWN_DATA) {
      return -1;
    }
  }

  int64_t total_flops = 0;
  ptr = cpu;

  for(int i=0; i < cpu->num_cpus; ptr = ptr->next_cpu, i++) {
    int vpus = get_number_of_vpus(ptr);
    int vpus_width = get_vpus_width(ptr);
    bool has_fma = has_fma_support(ptr);

    int64_t flops = ptr->topo->total_cores * get_freq(ptr->freq) * 1000000 * vpus * (vpus_width/32);
    if(has_fma) flops = flops * 2;

    total_flops += flops;
  }

  return total_flops;
}

uint32_t fill_ids_from_midr(uint32_t* midr_array, int32_t* freq_array, uint32_t* ids_array, int len) {
  uint32_t latest_id = 0;
  bool found;
  ids_array[0] = latest_id;

  for (int i = 1; i < len; i++) {
    int j = 0;
    found = false;

    for (j = 0; j < len && !found; j++) {
      if (i != j && cores_are_equal(i, j, midr_array, freq_array)) {
        if(j > i) {
          latest_id++;
          ids_array[i] = latest_id;
        }
        else {
          ids_array[i] = ids_array[j];
        }
        found = true;
      }
    }
    if(!found) {
      latest_id++;
      ids_array[i] = latest_id;
    }
  }

  return latest_id+1;
}

void init_cpu_info(struct cpuInfo* cpu) {
  cpu->next_cpu = NULL;
}

// We assume all cpus share the same hardware
// capabilities but I'm not sure it is always
// true...
// ARM32 https://elixir.bootlin.com/linux/latest/source/arch/arm/include/uapi/asm/hwcap.h
// ARM64 https://elixir.bootlin.com/linux/latest/source/arch/arm64/include/uapi/asm/hwcap.h
struct features* get_features_info(void) {
  struct features* feat = emalloc(sizeof(struct features));
  bool *ptr = &(feat->AES);
  for(uint32_t i = 0; i < sizeof(struct features)/sizeof(bool); i++, ptr++) {
    *ptr = false;
  }

#ifdef __linux__
  errno = 0;
  long hwcaps = getauxval(AT_HWCAP);

  if(errno == ENOENT) {
    printWarn("Unable to retrieve AT_HWCAP using getauxval");
  }
#ifdef __aarch64__
  else {
    feat->AES = hwcaps & HWCAP_AES;
    feat->CRC32 = hwcaps & HWCAP_CRC32;
    feat->SHA1 = hwcaps & HWCAP_SHA1;
    feat->SHA2 = hwcaps & HWCAP_SHA2;
    feat->NEON = hwcaps & HWCAP_ASIMD;
    feat->SVE = hwcaps & HWCAP_SVE;

    hwcaps = getauxval(AT_HWCAP2);
    if (errno == ENOENT) {
      printWarn("Unable to retrieve AT_HWCAP2 using getauxval");
    }
    else {
      #ifdef HWCAP2_SVE2
        feat->SVE2 = hwcaps & HWCAP2_SVE2;
      #endif
    }
  }
#else
  else {
    feat->NEON = hwcaps & HWCAP_NEON;
  }

  hwcaps = getauxval(AT_HWCAP2);
  if(errno == ENOENT) {
    printWarn("Unable to retrieve AT_HWCAP2 using getauxval");
  }
  else {
    feat->AES = hwcaps & HWCAP2_AES;
    feat->CRC32 = hwcaps & HWCAP2_CRC32;
    feat->SHA1 = hwcaps & HWCAP2_SHA1;
    feat->SHA2 = hwcaps & HWCAP2_SHA2;
    feat->SVE = false;
    feat->SVE2 = false;
  }
#endif // ifdef __aarch64__
#elif defined __APPLE__ || __MACH__
  // Must be M1
  feat->AES = true;
  feat->CRC32 = true;
  feat->SHA1 = true;
  feat->SHA2 = true;
  feat->NEON = true;
  feat->SVE = false;
  feat->SVE2 = false;
#elif defined _WIN32

  // CP 4020 maps to the ID_AA64PFR0_EL1 register on Windows
  // https://developer.arm.com/documentation/ddi0601/2024-06/AArch64-Registers/ID-AA64PFR0-EL1--AArch64-Processor-Feature-Register-0
  int64_t pfr0 = 0;
  if(!get_win32_core_info_int(0, "CP 4020", &pfr0, true)) {
    printWarn("Unable to retrieve PFR0 via registry");
  }
  else {
    // AdvSimd[23:20]
    // -1: Not available
    //  0: AdvSimd support
    //  1: AdvSimd support + FP16
    int8_t adv_simd = ((int64_t)(pfr0 << (60 - 20)) >> 60);
    feat->NEON = (adv_simd >= 0);

    // SVE[35:32]
    feat->SVE = (pfr0 >> 32) & 0xF ? true : false;
  }

  // Windoes does not expose a registry entry for the ID_AA64ZFR0_EL1 register
  // this would have mapped to "CP 4024".
  feat->SVE2 = false;

  // CP 4030 maps to the ID_AA64ISAR0_EL1 register on Windows
  // https://developer.arm.com/documentation/ddi0601/2024-06/AArch64-Registers/ID-AA64ISAR0-EL1--AArch64-Instruction-Set-Attribute-Register-0
  int64_t isar0 = 0;
  if(!get_win32_core_info_int(0, "CP 4030", &isar0, true)) {
    printWarn("Unable to retrieve ISAR0 via registry");
  }
  else {
    // AES[7:4]
    feat->AES = (isar0 >> 4) & 0xF ? true : false;
    // SHA1[11:8]
    feat->SHA1 = (isar0 >> 8) & 0xF ? true : false;
    // SHA2[15:12]
    feat->SHA2 = (isar0 >> 12) & 0xF ? true : false;
    // CRC32[19:16]
    feat->CRC32 = (isar0 >> 16) & 0xF ? true : false;
  }
#endif  // ifdef __linux__

  if (feat->SVE || feat->SVE2) {
    feat->cntb = sve_cntb();
  }

  return feat;
}

#ifdef __linux__
struct cpuInfo* get_cpu_info_linux(struct cpuInfo* cpu) {
  init_cpu_info(cpu);
  int ncores = get_ncores_from_cpuinfo();
  bool success = false;
  int32_t* freq_array = emalloc(sizeof(uint32_t) * ncores);
  uint32_t* midr_array = emalloc(sizeof(uint32_t) * ncores);
  uint32_t* ids_array = emalloc(sizeof(uint32_t) * ncores);

  for(int i=0; i < ncores; i++) {
    midr_array[i] = get_midr_from_cpuinfo(i, &success);

    if(!success) {
      printWarn("Unable to fetch MIDR for core %d. This is probably because the core is offline", i);
      midr_array[i] = midr_array[0];
    }

    freq_array[i] = get_max_freq_from_file(i);
    if(freq_array[i] == UNKNOWN_DATA) {
      printWarn("Unable to fetch max frequency for core %d. This is probably because the core is offline", i);
      freq_array[i] = freq_array[0];
    }
  }
  uint32_t sockets = fill_ids_from_midr(midr_array, freq_array, ids_array, ncores);

  struct cpuInfo* ptr = cpu;
  int midr_idx = 0;
  int tmp_midr_idx = 0;
  for(uint32_t i=0; i < sockets; i++) {
    if(i > 0) {
      ptr->next_cpu = emalloc(sizeof(struct cpuInfo));
      ptr = ptr->next_cpu;
      init_cpu_info(ptr);

      tmp_midr_idx = midr_idx;
      while(cores_are_equal(midr_idx, tmp_midr_idx, midr_array, freq_array)) tmp_midr_idx++;
      midr_idx = tmp_midr_idx;
    }

    ptr->midr = midr_array[midr_idx];
    ptr->arch = get_uarch_from_midr(ptr->midr, ptr);

    ptr->feat = get_features_info();
    ptr->freq = get_frequency_info(midr_idx);
    ptr->cach = get_cache_info(ptr);
    ptr->topo = get_topology_info(ptr, ptr->cach, midr_array, freq_array, i, ncores);
  }

  cpu->num_cpus = sockets;
  cpu->hv = emalloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  cpu->soc = get_soc(cpu);
  cpu->peak_performance = get_peak_performance(cpu);

  return cpu;
}

#elif defined __APPLE__ || __MACH__
void fill_cpu_info_firestorm_icestorm(struct cpuInfo* cpu, uint32_t pcores, uint32_t ecores) {
  // 1. Fill ICESTORM
  struct cpuInfo* ice = cpu;

  ice->midr = MIDR_APPLE_M1_ICESTORM;
  ice->arch = get_uarch_from_midr(ice->midr, ice);
  ice->cach = get_cache_info(ice);
  ice->feat = get_features_info();
  ice->topo = malloc(sizeof(struct topology));
  ice->topo->cach = ice->cach;
  ice->topo->total_cores = ecores;
  ice->freq = malloc(sizeof(struct frequency));
  ice->freq->base = UNKNOWN_DATA;
  ice->freq->max = 2064;
  ice->hv = malloc(sizeof(struct hypervisor));
  ice->hv->present = false;
  ice->next_cpu = malloc(sizeof(struct cpuInfo));

  // 2. Fill FIRESTORM
  struct cpuInfo* fire = ice->next_cpu;
  fire->midr = MIDR_APPLE_M1_FIRESTORM;
  fire->arch = get_uarch_from_midr(fire->midr, fire);
  fire->cach = get_cache_info(fire);
  fire->feat = get_features_info();
  fire->topo = malloc(sizeof(struct topology));
  fire->topo->cach = fire->cach;
  fire->topo->total_cores = pcores;
  fire->freq = malloc(sizeof(struct frequency));
  fire->freq->base = UNKNOWN_DATA;
  fire->freq->max = 3200;
  fire->hv = malloc(sizeof(struct hypervisor));
  fire->hv->present = false;
  fire->next_cpu = NULL;
}

void fill_cpu_info_avalanche_blizzard(struct cpuInfo* cpu, uint32_t pcores, uint32_t ecores) {
  // 1. Fill BLIZZARD
  struct cpuInfo* bli = cpu;

  bli->midr = MIDR_APPLE_M2_BLIZZARD;
  bli->arch = get_uarch_from_midr(bli->midr, bli);
  bli->cach = get_cache_info(bli);
  bli->feat = get_features_info();
  bli->topo = malloc(sizeof(struct topology));
  bli->topo->cach = bli->cach;
  bli->topo->total_cores = ecores;
  bli->freq = malloc(sizeof(struct frequency));
  bli->freq->base = UNKNOWN_DATA;
  bli->freq->max = 2800;
  bli->hv = malloc(sizeof(struct hypervisor));
  bli->hv->present = false;
  bli->next_cpu = malloc(sizeof(struct cpuInfo));

  // 2. Fill AVALANCHE
  struct cpuInfo* ava = bli->next_cpu;
  ava->midr = MIDR_APPLE_M2_AVALANCHE;
  ava->arch = get_uarch_from_midr(ava->midr, ava);
  ava->cach = get_cache_info(ava);
  ava->feat = get_features_info();
  ava->topo = malloc(sizeof(struct topology));
  ava->topo->cach = ava->cach;
  ava->topo->total_cores = pcores;
  ava->freq = malloc(sizeof(struct frequency));
  ava->freq->base = UNKNOWN_DATA;
  ava->freq->max = 3500;
  ava->hv = malloc(sizeof(struct hypervisor));
  ava->hv->present = false;
  ava->next_cpu = NULL;
}

void fill_cpu_info_everest_sawtooth(struct cpuInfo* cpu, uint32_t pcores, uint32_t ecores) {
  // 1. Fill SAWTOOTH
  struct cpuInfo* saw = cpu;

  saw->midr = MIDR_APPLE_M3_SAWTOOTH;
  saw->arch = get_uarch_from_midr(saw->midr, saw);
  saw->cach = get_cache_info(saw);
  saw->feat = get_features_info();
  saw->topo = malloc(sizeof(struct topology));
  saw->topo->cach = saw->cach;
  saw->topo->total_cores = ecores;
  saw->freq = malloc(sizeof(struct frequency));
  saw->freq->base = UNKNOWN_DATA;
  saw->freq->max = 2750;
  saw->hv = malloc(sizeof(struct hypervisor));
  saw->hv->present = false;
  saw->next_cpu = malloc(sizeof(struct cpuInfo));

  // 2. Fill EVEREST
  struct cpuInfo* eve = saw->next_cpu;
  eve->midr = MIDR_APPLE_M3_EVEREST;
  eve->arch = get_uarch_from_midr(eve->midr, eve);
  eve->cach = get_cache_info(eve);
  eve->feat = get_features_info();
  eve->topo = malloc(sizeof(struct topology));
  eve->topo->cach = eve->cach;
  eve->topo->total_cores = pcores;
  eve->freq = malloc(sizeof(struct frequency));
  eve->freq->base = UNKNOWN_DATA;
  eve->freq->max = 4050;
  eve->hv = malloc(sizeof(struct hypervisor));
  eve->hv->present = false;
  eve->next_cpu = NULL;
}

struct cpuInfo* get_cpu_info_mach(struct cpuInfo* cpu) {
  // https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_system_capabilities
  uint32_t nperflevels = get_sys_info_by_name("hw.nperflevels");

  if((cpu->num_cpus = nperflevels) != 2) {
    printBug("Expected to find SoC with 2 perf levels, found: %d", cpu->num_cpus);
    return NULL;
  }

  uint32_t pcores = get_sys_info_by_name("hw.perflevel0.physicalcpu");
  uint32_t ecores = get_sys_info_by_name("hw.perflevel1.physicalcpu");
  if(ecores <= 0) {
    printBug("Expected to find a numer of ecores > 0, found: %d", ecores);
    return NULL;
  }
  if(pcores <= 0) {
    printBug("Expected to find a numer of pcores > 0, found: %d", pcores);
    return NULL;
  }

  uint32_t cpu_family = get_sys_info_by_name("hw.cpufamily");
  // Manually fill the cpuInfo assuming that
  // the CPU is an Apple SoC
  if(cpu_family == CPUFAMILY_ARM_FIRESTORM_ICESTORM) {
    fill_cpu_info_firestorm_icestorm(cpu, pcores, ecores);
    cpu->soc = get_soc(cpu);
    cpu->peak_performance = get_peak_performance(cpu);
  }
  else if(cpu_family == CPUFAMILY_ARM_AVALANCHE_BLIZZARD) {
    fill_cpu_info_avalanche_blizzard(cpu, pcores, ecores);
    cpu->soc = get_soc(cpu);
    cpu->peak_performance = get_peak_performance(cpu);
  }
  else if(cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH ||
          cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH_2   ||
          cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH_PRO ||
          cpu_family == CPUFAMILY_ARM_EVEREST_SAWTOOTH_MAX) {
    fill_cpu_info_everest_sawtooth(cpu, pcores, ecores);
    cpu->soc = get_soc(cpu);
    cpu->peak_performance = get_peak_performance(cpu);
  }
  else {
    printBugCheckRelease("Found invalid cpu_family: 0x%.8X", cpu_family);
    return NULL;
  }

  return cpu;
}
#elif defined _WIN32
struct cpuInfo* get_cpu_info_windows(struct cpuInfo* cpu) {
  init_cpu_info(cpu);

  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  int ncores = sys_info.dwNumberOfProcessors;

  uint32_t* midr_array = emalloc(sizeof(uint32_t) * ncores);
  int32_t* freq_array = emalloc(sizeof(uint32_t) * ncores);
  uint32_t* ids_array = emalloc(sizeof(uint32_t) * ncores);
  for(int i=0; i < ncores; i++) {
    // Cast from 64 to 32 bit to be able to re-use the pre-existing
    // functions such as fill_ids_from_midr and cores_are_equal
    int64_t midr_64;
    if(!get_win32_core_info_int(i, "CP 4000", &midr_64, true)) {
      return NULL;
    }
    midr_array[i] = midr_64;
    if(!get_win32_core_info_int(i, "~MHz", &freq_array[i], false)) {
      return NULL;
    }
  }

  uint32_t sockets = fill_ids_from_midr(midr_array, freq_array, ids_array, ncores);

  struct cpuInfo* ptr = cpu;
  int midr_idx = 0;
  int tmp_midr_idx = 0;
  for(uint32_t i=0; i < sockets; i++) {
    if(i > 0) {
      ptr->next_cpu = emalloc(sizeof(struct cpuInfo));
      ptr = ptr->next_cpu;
      init_cpu_info(ptr);

      tmp_midr_idx = midr_idx;
      while(cores_are_equal(midr_idx, tmp_midr_idx, midr_array, freq_array)) tmp_midr_idx++;
      midr_idx = tmp_midr_idx;
    }

    ptr->midr = midr_array[midr_idx];
    ptr->arch = get_uarch_from_midr(ptr->midr, ptr);

    ptr->feat = get_features_info();
    
    ptr->freq = emalloc(sizeof(struct frequency));
    ptr->freq->measured = false;
    ptr->freq->base = freq_array[midr_idx];
    ptr->freq->max = UNKNOWN_DATA;

    ptr->cach = get_cache_info(ptr);
    ptr->topo = get_topology_info(ptr, ptr->cach, midr_array, freq_array, i, ncores);
  }

  cpu->num_cpus = sockets;
  cpu->hv = emalloc(sizeof(struct hypervisor));
  cpu->hv->present = false;
  cpu->soc = get_soc(cpu);
  cpu->peak_performance = get_peak_performance(cpu);

  return cpu;
}
#endif

struct cpuInfo* get_cpu_info(void) {
  struct cpuInfo* cpu = malloc(sizeof(struct cpuInfo));
  init_cpu_info(cpu);

  #ifdef __linux__
    return get_cpu_info_linux(cpu);
  #elif defined __APPLE__ || __MACH__
    return get_cpu_info_mach(cpu);
  #elif defined _WIN32
    return get_cpu_info_windows(cpu);
  #endif
}

char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket) {
  uint32_t size = 3+7+1;
  char*  string = emalloc(sizeof(char)*size);
  snprintf(string, size, "%d cores", topo->total_cores);

  return string;
}

char* get_str_features(struct cpuInfo* cpu) {
  struct features* feat = cpu->feat;
  uint32_t max_len = strlen("NEON,SHA1,SHA2,AES,CRC32,SVE,SVE2,") + 1;
  uint32_t len = 0;
  char* string = ecalloc(max_len, sizeof(char));

  if(feat->NEON) {
    strcat(string, "NEON,");
    len += 5;
  }
  if(feat->SVE) {
    strcat(string, "SVE,");
    len += 4;
  }
  if(feat->SVE2) {
    strcat(string, "SVE2,");
    len += 5;
  }
  if(feat->SHA1) {
    strcat(string, "SHA1,");
    len += 5;
  }
  if(feat->SHA2) {
    strcat(string, "SHA2,");
    len += 5;
  }
  if(feat->AES) {
    strcat(string, "AES,");
    len += 4;
  }
  if(feat->CRC32) {
    strcat(string, "CRC32,");
    len += 6;
  }

  if(len > 0) {
    string[len-1] = '\0';
    return string;
  }
  else
    return NULL;
}

void print_debug(struct cpuInfo* cpu) {
  int ncores = get_ncores_from_cpuinfo();
  bool success = false;

  for(int i=0; i < ncores; i++) {
    printf("[Core %d] ", i);
    long freq = get_max_freq_from_file(i);
    uint32_t midr = get_midr_from_cpuinfo(i, &success);
    if(!success) {
      printWarn("Unable to fetch MIDR for core %d. This is probably because the core is offline", i);
      printf("0x%.8X ", get_midr_from_cpuinfo(0, &success));
    }
    else {
      printf("0x%.8X ", midr);
    }
    if(freq == UNKNOWN_DATA) {
      printWarn("Unable to fetch max frequency for core %d. This is probably because the core is offline", i);
      printf("%ld MHz\n", get_max_freq_from_file(0));
    }
    else {
      printf("%ld MHz\n", freq);
    }
  }

  if (cpu->feat->SVE || cpu->feat->SVE2) {
    printf("- cntb: %d\n", (int) cpu->feat->cntb);
  }

  #if defined(__APPLE__) || defined(__MACH__)
    printf("hw.cpufamily: 0x%.8X\n", get_sys_info_by_name("hw.cpufamily"));
    printf("hw.cpusubfamily: 0x%.8X\n", get_sys_info_by_name("hw.cpusubfamily"));
    printf("hw.nperflevels: %d\n", get_sys_info_by_name("hw.nperflevels"));
    printf("hw.physicalcpu: %d\n", get_sys_info_by_name("hw.physicalcpu"));
    printf("hw.perflevel0.physicalcpu: %d\n", get_sys_info_by_name("hw.perflevel0.physicalcpu"));
    printf("hw.perflevel1.physicalcpu: %d\n", get_sys_info_by_name("hw.perflevel1.physicalcpu"));
  #endif
}

void free_topo_struct(struct topology* topo) {
  free(topo);
}
