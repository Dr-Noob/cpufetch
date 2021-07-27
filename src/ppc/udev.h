#ifndef __UDEV_PPC__
#define __UDEV_PPC__

#include "../common/udev.h"

bool fill_core_ids_from_sys(int *core_ids, int total_cores);
bool fill_package_ids_from_sys(int* package_ids, int total_cores);

#endif
