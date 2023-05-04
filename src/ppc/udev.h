#ifndef __UDEV_PPC__
#define __UDEV_PPC__

#include "../common/udev.h"
#define _PATH_DT                  "/proc/device-tree/vpd/root-node-vpd@a000/enclosure@1e00/backplane@800/processor@1000"
#define _PATH_DT_PART             "/part-number"
#define _PATH_DT_IBM_PARTIT_NAME  "/device-tree/ibm,partition-name"
#define _PATH_DT_HMC_MANAGED      "/device-tree/hmc-managed?"
#define _PATH_DT_QEMU_WIDTH       "/device-tree/chosen/qemu,graphic-width"

bool fill_core_ids_from_sys(int *core_ids, int total_cores);
bool fill_package_ids_from_sys(int* package_ids, int total_cores);
int get_num_sockets_package_cpus(struct topology* topo);
long get_frequency_from_cpuinfo(void);

#endif
