#include <errno.h>
#include <string.h>

#include "../common/global.h"
#include "udev.h"

#define _PATH_DEVTREE "/proc/device-tree/compatible"
#define DEVTREE_HARDWARE_FIELD 0

char* get_field_from_devtree(int DEVTREE_FIELD) {
  int filelen;
  char* buf;
  if((buf = read_file(_PATH_DEVTREE, &filelen)) == NULL) {
    printWarn("read_file: %s: %s", _PATH_DEVTREE, strerror(errno));
    return NULL;
  }

  // Here we would use strstr to find the comma.
  // However, the device-tree file may contain NULL
  // bytes in the middle of the string, which would
  // cause strstr to return NULL even when there might
  // be an occurence after the NULL byte
  //
  // We iterate the string backwards to find the field
  // in position n-DEVTREE_HARDWARE_FIELD where n
  // is the number of fields.
  int i=0;
  char* tmp1 = buf+filelen-1;
  do {
    tmp1--;
    if(*tmp1 == ',') i++;
  } while(tmp1 != buf && i <= DEVTREE_FIELD);

  if(tmp1 == buf) {
    printWarn("get_field_from_devtree: Unable to find field %d", DEVTREE_FIELD);
    return NULL;
  }

  tmp1++;
  int strlen = filelen-(tmp1-buf);
  char* hardware = emalloc(sizeof(char) * strlen);
  memset(hardware, 0, sizeof(char) * strlen);
  strncpy(hardware, tmp1, strlen-1);

  return hardware;
}

char* get_hardware_from_devtree(void) {
  return get_field_from_devtree(DEVTREE_HARDWARE_FIELD);
}
