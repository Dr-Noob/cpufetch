#define _GNU_SOURCE

#include <sys/stat.h>
#include <dirent.h>

#include "udev.h"
#include "global.h"
#include "pci.h"

#ifndef PATH_MAX
  #define PATH_MAX 1024
#endif

#define PCI_PATH "/sys/bus/pci/devices/"
#define MAX_LENGTH_PCI_DIR_NAME 1024

/*
 * doc: https://wiki.osdev.org/PCI#Class_Codes
 *      https://pci-ids.ucw.cz/read/PC
 */
#define PCI_VENDOR_ID_AMD    0x1002
#define CLASS_VGA_CONTROLLER 0x0300
#define CLASS_3D_CONTROLLER  0x0302

// Return a list of PCI devices containing only
// the sysfs path
struct pci_devices * get_pci_paths(void) {
  DIR *dirp;

  if ((dirp = opendir(PCI_PATH)) == NULL) {
    perror("opendir");
    return NULL;
  }

  struct dirent *dp;
  int numDirs = 0;  
  errno = 0;

  while ((dp = readdir(dirp)) != NULL) {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
      numDirs++;
  }  
  if (errno != 0) {
    perror("readdir");
    return NULL;
  }

  rewinddir(dirp);

  struct pci_devices * pci = emalloc(sizeof(struct pci_devices));
  pci->num_devices = numDirs;
  pci->devices = emalloc(sizeof(struct pci_device) * pci->num_devices);
  char * full_path = emalloc(PATH_MAX * sizeof(char));
  struct stat stbuf;
  int i = 0;

  while ((dp = readdir(dirp)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
      continue;

    if (strlen(dp->d_name) > MAX_LENGTH_PCI_DIR_NAME) {
      printErr("Directory name is too long: %s", dp->d_name);
      return NULL;
    }

    memset(full_path, 0, PATH_MAX * sizeof(char));
    snprintf(full_path, min(strlen(PCI_PATH) + strlen(dp->d_name) + 1, PATH_MAX), "%s%s", PCI_PATH, dp->d_name);    

    if (stat(full_path, &stbuf) == -1) {
      perror("stat");
      return NULL;
    }

    if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
      int strLen = min(MAX_LENGTH_PCI_DIR_NAME, strlen(dp->d_name)) + 1;
      pci->devices[i] = emalloc(sizeof(struct pci_device));
      pci->devices[i]->path = ecalloc(sizeof(char), strLen);
      strncpy(pci->devices[i]->path, dp->d_name, strLen);
      i++;
    }        
  }
  if (errno != 0) {
    perror("readdir");
    return NULL;
  }

  return pci;
}

// For each PCI device in the list pci, fetch its vendor and 
// device id using sysfs (e.g., /sys/bus/pci/devices/XXX/{vendor/device})
void populate_pci_devices(struct pci_devices * pci) {
  int filelen;
  char* buf;

  for (int i=0; i < pci->num_devices; i++) {
    struct pci_device* dev = pci->devices[i];
    int path_size = strlen(PCI_PATH) + strlen(dev->path) + 2;

    // Read vendor_id
    char *vendor_id_path = emalloc(sizeof(char) * (path_size + strlen("vendor") + 1));
    sprintf(vendor_id_path, "%s/%s/%s", PCI_PATH, dev->path, "vendor");

    if ((buf = read_file(vendor_id_path, &filelen)) == NULL) {
      printWarn("read_file: %s: %s\n", vendor_id_path, strerror(errno));
      dev->vendor_id = 0;
    }
    else {
      dev->vendor_id = strtol(buf, NULL, 16);
    }

    // Read device_id
    char *device_id_path = emalloc(sizeof(char) * (path_size + strlen("device") + 1));
    sprintf(device_id_path, "%s/%s/%s", PCI_PATH, dev->path, "device");

    if ((buf = read_file(device_id_path, &filelen)) == NULL) {
      printWarn("read_file: %s: %s\n", device_id_path, strerror(errno));
      dev->device_id = 0;
    }
    else {
      dev->device_id = strtol(buf, NULL, 16);
    }

    free(vendor_id_path);
    free(device_id_path);
  }
}

// Right now, we are interested in PCI devices which
// vendor is NVIDIA (to be extended in the future).
// Should we also restrict to VGA controllers only?
bool pci_device_is_useful(struct pci_device* dev) {
  return dev->vendor_id == PCI_VENDOR_NVIDIA;
}

// Filter the input list in order to get only those PCI devices which 
// we are interested in (decided by pci_device_is_useful)
// and return the filtered result.
struct pci_devices * filter_pci_devices(struct pci_devices * pci) {
  int * devices_to_get = emalloc(sizeof(int) * pci->num_devices);
  int dev_ptr = 0;

  for (int i=0; i < pci->num_devices; i++) {
    if (pci_device_is_useful(pci->devices[i])) {
      devices_to_get[dev_ptr] = i;
      dev_ptr++;
    }
  }

  struct pci_devices * pci_filtered = emalloc(sizeof(struct pci_devices));
  pci_filtered->num_devices = dev_ptr;

  if (pci_filtered->num_devices == 0) {
    pci_filtered->devices = NULL;
  }
  else {
    pci_filtered->devices = emalloc(sizeof(struct pci_device) * pci_filtered->num_devices);  

    for (int i=0; i < pci_filtered->num_devices; i++)
      pci_filtered->devices[i] = pci->devices[devices_to_get[i]];
  }

  return pci_filtered;
}

// Return a list of PCI devices that could be used to infer the SoC.
// The criteria to determine which devices are suitable for this task
// is decided in filter_pci_devices.
struct pci_devices * get_pci_devices(void) {
  struct pci_devices * pci = get_pci_paths();

  if (pci == NULL)
    return NULL;

  populate_pci_devices(pci);  

  return filter_pci_devices(pci);
}
