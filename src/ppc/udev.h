#ifndef __UDEV_PPC__
#define __UDEV_PPC__

#include "../common/udev.h"
#define _PATH_DT              "/proc/device-tree/vpd/root-node-vpd@a000/enclosure@1e00/backplane@800/processor@1000"
#define _PATH_DT_PART         "/part-number"

bool fill_core_ids_from_sys(int *core_ids, int total_cores);
bool fill_package_ids_from_sys(int* package_ids, int total_cores);

#endif
