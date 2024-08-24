#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "ppc.h"
#include "uarch.h"
#include "udev.h"
#include "../common/udev.h"
#include "../common/global.h"

static char *hv_vendors_name[] = {
  [HV_VENDOR_KVM]       = "KVM",
  [HV_VENDOR_QEMU]      = "QEMU",
  [HV_VENDOR_VBOX]      = "VirtualBox",
  [HV_VENDOR_HYPERV]    = "Microsoft Hyper-V",
  [HV_VENDOR_VMWARE]    = "VMware",
  [HV_VENDOR_XEN]       = "Xen",
  [HV_VENDOR_PARALLELS] = "Parallels",
  [HV_VENDOR_PHYP]      = "pHyp",
  [HV_VENDOR_BHYVE]     = "bhyve",
  [HV_VENDOR_APPLEVZ]   = "Apple VZ",
  [HV_VENDOR_INVALID]   = STRING_UNKNOWN
};

struct cache* get_cache_info(struct cpuInfo* cpu) {
  struct cache* cach = emalloc(sizeof(struct cache));
  init_cache_struct(cach);

  cach->L1i->size = get_l1i_cache_size(0);
  cach->L1d->size = get_l1d_cache_size(0);
  cach->L2->size = get_l2_cache_size(0);
  cach->L3->size = get_l3_cache_size(0);

  if(cach->L1i->size > 0) {
    cach->L1i->exists = true;
    cach->L1i->num_caches = get_num_caches_by_level(cpu, 0);
    cach->max_cache_level = 1;
  }
  if(cach->L1d->size > 0) {
    cach->L1d->exists = true;
    cach->L1d->num_caches = get_num_caches_by_level(cpu, 1);
    cach->max_cache_level = 2;
  }
  if(cach->L2->size > 0) {
    cach->L2->exists = true;
    cach->L2->num_caches = get_num_caches_by_level(cpu, 2);
    cach->max_cache_level = 3;
  }
  if(cach->L3->size > 0) {
    cach->L3->exists = true;
    cach->L3->num_caches = get_num_caches_by_level(cpu, 3);
    cach->max_cache_level = 4;
  }

  return cach;
}

struct topology* get_topology_info(struct cache* cach) {
  struct topology* topo = emalloc(sizeof(struct topology));
  init_topology_struct(topo, cach);

  // 1. Total cores detection
  if((topo->total_cores = sysconf(_SC_NPROCESSORS_ONLN)) == -1) {
    printWarn("sysconf(_SC_NPROCESSORS_ONLN): %s", strerror(errno));
    topo->total_cores = 1; // fallback
  }

  // To find physical cores, we use topo->total_cores and core_ids
  // To find number of sockets, we use package_ids
  int* core_ids = emalloc(sizeof(int) * topo->total_cores);
  int* package_ids = emalloc(sizeof(int) * topo->total_cores);

  if(!fill_core_ids_from_sys(core_ids, topo->total_cores)) {
    printWarn("fill_core_ids_from_sys failed, output may be incomplete/invalid");
    for(int i=0; i < topo->total_cores; i++) core_ids[i] = 0;
  }

  if(!fill_package_ids_from_sys(package_ids, topo->total_cores)) {
    printWarn("fill_package_ids_from_sys failed, output may be incomplete/invalid");
    for(int i=0; i < topo->total_cores; i++) package_ids[i] = 0;
    // fill_package_ids_from_sys failed, use a
    // more sophisticated wat to find the number of sockets
    topo->sockets = get_num_sockets_package_cpus(topo);
    if (topo->sockets == UNKNOWN_DATA) {
      printWarn("get_num_sockets_package_cpus failed: assuming 1 socket");
      topo->sockets = 1;
    }
  }
  else {
    // fill_package_ids_from_sys succeeded, use the
    // traditional socket detection algorithm
    int *package_ids_count = emalloc(sizeof(int) * topo->total_cores);
    for(int i=0; i < topo->total_cores; i++) {
      package_ids_count[i] = 0;
    }
    for(int i=0; i < topo->total_cores; i++) {
      package_ids_count[package_ids[i]]++;
    }
    for(int i=0; i < topo->total_cores; i++) {
      if(package_ids_count[i] != 0) {
        topo->sockets++;
      }
    }
    free(package_ids_count);
  }

  // 3. Physical cores detection
  int *core_ids_unified = emalloc(sizeof(int) * topo->total_cores);
  for(int i=0; i < topo->total_cores; i++) {
    core_ids_unified[i] = -1;
  }
  bool found = false;
  for(int i=0; i < topo->total_cores; i++) {
    for(int j=0; j < topo->total_cores && !found; j++) {
      if(core_ids_unified[j] == core_ids[i]) found = true;
    }
    if(!found) {
      core_ids_unified[topo->physical_cores] = core_ids[i];
      topo->physical_cores++;
    }
    found = false;
  }

  topo->physical_cores = topo->physical_cores / topo->sockets; // only count cores on one socket
  topo->logical_cores = topo->total_cores / topo->sockets;     // only count threads on one socket
  topo->smt_supported = topo->logical_cores / topo->physical_cores;

  free(core_ids);
  free(package_ids);
  free(core_ids_unified);

  return topo;
}

static inline uint32_t mfpvr(void) {
    uint32_t pvr;

    asm ("mfpvr %0"
         : "=r"(pvr));
    return pvr;
}

struct uarch* get_cpu_uarch(struct cpuInfo* cpu) {
  return get_uarch_from_pvr(cpu->pvr);
}

struct frequency* get_frequency_info(void) {
  struct frequency* freq = emalloc(sizeof(struct frequency));

  freq->max = get_max_freq_from_file(0);
  freq->base = get_min_freq_from_file(0);

  if(freq->max == UNKNOWN_DATA) {
    // If we are unable to find it in the
    // standard path, try /proc/cpuinfo
    freq->max = get_frequency_from_cpuinfo();
  }

  return freq;
}

int64_t get_peak_performance(struct cpuInfo* cpu, struct topology* topo, int64_t freq) {
  /*
   * Not sure about this
   * PP(SP) = N_CORES * FREQUENCY * 4(If altivec)
   */

  //First check we have consistent data
  if(freq == UNKNOWN_DATA) {
    return -1;
  }

  struct features* feat = cpu->feat;
  int64_t flops = topo->physical_cores * topo->sockets * (freq * 1000000);
  if(feat->altivec) flops = flops * 4;

  // POWER9 has the concept called "slices". Each SMT4 core has two super-slices,
  // and each super-slice is capable of doing two FLOPS per cycle. In the case of
  // SMT8, it has 4 super-slices, thus four FLOPS per cycle.
  if(is_power9(cpu->arch)) {
    int threads_per_core = topo->logical_cores / topo->physical_cores;
    flops = flops * (threads_per_core / 2);
  }

  return flops;
}

struct hypervisor* get_hp_info(void) {
  struct hypervisor* hv = emalloc(sizeof(struct hypervisor));
  hv->present = false;

  // Weird heuristic found in lscpu:
  // https://github.com/util-linux/util-linux/blob/master/sys-utils/lscpu-virt.c
  if(access("/proc" _PATH_DT_IBM_PARTIT_NAME, F_OK) == 0 &&
     access("/proc" _PATH_DT_HMC_MANAGED, F_OK) == 0 &&
     access("/proc" _PATH_DT_QEMU_WIDTH, F_OK) != 0) {
    hv->present = true;
    hv->hv_vendor = HV_VENDOR_PHYP;
  }
  else if(is_devtree_compatible("qemu,pseries")) {
    hv->present = true;
    hv->hv_vendor = HV_VENDOR_QEMU;
  }

  hv->hv_name = hv_vendors_name[hv->hv_vendor];

  return hv;
}

struct cpuInfo* get_cpu_info(void) {
  struct cpuInfo* cpu = emalloc(sizeof(struct cpuInfo));
  struct features* feat = emalloc(sizeof(struct features));
  cpu->feat = feat;

  bool *ptr = &(feat->AES);
  for(uint32_t i = 0; i < sizeof(struct features)/sizeof(bool); i++, ptr++) {
    *ptr = false;
  }

  int len;
  char* path = emalloc(sizeof(char) * (strlen(_PATH_DT) + strlen(_PATH_DT_PART) + 1));
  sprintf(path, "%s%s", _PATH_DT, _PATH_DT_PART);

  if((cpu->cpu_name = read_file(path, &len)) == NULL) {
    printWarn("Could not open '%s'", path);
  }
  cpu->pvr = mfpvr();
  cpu->hv = get_hp_info();
  cpu->arch = get_cpu_uarch(cpu);
  cpu->freq = get_frequency_info();
  cpu->topo = get_topology_info(cpu->cach);
  cpu->cach = get_cache_info(cpu);
  feat->altivec = has_altivec(cpu->arch);
  cpu->peak_performance = get_peak_performance(cpu, cpu->topo, get_freq(cpu->freq));

  return cpu;
}

char* get_str_altivec(struct cpuInfo* cpu) {
  char* string = ecalloc(4, sizeof(char));

  if(cpu->feat->altivec) strcpy(string, "Yes");
  else strcpy(string, "No");

  return string;
}

char* get_str_topology(struct topology* topo, bool dual_socket) {
  char* string;
  if(topo->smt_supported > 1) {
    uint32_t size = 3+3+17+1;
    string = emalloc(sizeof(char)*size);
    if(dual_socket)
      snprintf(string, size, "%d cores (%d threads)", topo->physical_cores * topo->sockets, topo->logical_cores * topo->sockets);
    else
      snprintf(string, size, "%d cores (%d threads)",topo->physical_cores,topo->logical_cores);
  }
  else {
    uint32_t size = 3+7+1;
    string = emalloc(sizeof(char)*size);
    if(dual_socket)
      snprintf(string, size, "%d cores",topo->physical_cores * topo->sockets);
    else
      snprintf(string, size, "%d cores",topo->physical_cores);
  }
  return string;
}


void print_debug(struct cpuInfo* cpu) {
  printf("PVR: 0x%.8X\n", cpu->pvr);
}
