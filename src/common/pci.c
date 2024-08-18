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
    char *vendor_id_path = emalloc(sizeof(char) * (path_size + strlen("vendor")));
    sprintf(vendor_id_path, "%s/%s/%s", PCI_PATH, dev->path, "vendor");
    
    if ((buf = read_file(vendor_id_path, &filelen)) == NULL) {
      printWarn("read_file: %s: %s\n", vendor_id_path, strerror(errno));
      dev->vendor_id = 0;
    }
    else {
      dev->vendor_id = strtol(buf, NULL, 16);
    }

    // Read device_id
    char *device_id_path = emalloc(sizeof(char) * (path_size + strlen("device")));
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

// Return a list of PCI devices that could be used to infer the SoC.
// The criteria to determine which devices are suitable for this task
// is decided in filter_pci_devices.
struct pci_devices * get_pci_devices(void) {
  struct pci_devices * pci = get_pci_paths();

  if (pci == NULL)
    return NULL;

  populate_pci_devices(pci);  

  return pci;
}
