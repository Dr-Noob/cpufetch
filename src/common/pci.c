#define _GNU_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>

#include <stdbool.h>

#include "udev.h"
#include "global.h"
#include "pci.h"

// TODO: Linux only
// TODO: Only neccesary headers (dont remove limits)

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

struct pci_devices * get_pci_paths(void) {
  DIR *dfd;

  if ((dfd = opendir(PCI_PATH)) == NULL) {
    perror("opendir");
    return false;
  }

  struct dirent *dp;
  int numDirs = 0;

  // TODO: error? readdir
  while ((dp = readdir(dfd)) != NULL) {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
      numDirs++;
  }

  rewinddir(dfd);

  // char ** pciDirs = emalloc(numDirs * sizeof(char *));
  struct pci_devices * pci = emalloc(sizeof(struct pci_devices));
  pci->num_devices = numDirs;
  pci->devices = emalloc(sizeof(struct pci_device) * pci->num_devices);
  char * full_path = emalloc(PATH_MAX * sizeof(char *));
  struct stat stbuf;
  int i = 0;

  while ((dp = readdir(dfd)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
      continue;

    if (strlen(dp->d_name) > MAX_LENGTH_PCI_DIR_NAME) {
      printErr("Directory name is too long: %s", dp->d_name);
      return false;
    }

    memset(full_path, 0, PATH_MAX * sizeof(char *));
    snprintf(full_path, min(strlen(PCI_PATH) + strlen(dp->d_name) + 1, PATH_MAX), "%s%s", PCI_PATH, dp->d_name);    

    if (stat(full_path, &stbuf ) == -1) {
      perror("stat");
      return false;
    }

    if ((stbuf.st_mode & S_IFMT ) == S_IFDIR) {
      int strLen = min(MAX_LENGTH_PCI_DIR_NAME, strlen(dp->d_name)) + 1;
      pci->devices[i] = emalloc(sizeof(struct pci_device));
      pci->devices[i]->path = ecalloc(sizeof(char), strLen);
      strncpy(pci->devices[i]->path, dp->d_name, strLen);
      i++;
    }        
  }

  return pci;
}

void populate_pci_devices(struct pci_devices * pci) {
  int filelen;
  char* buf;

  for (int i=0; i < pci->num_devices; i++) {
    struct pci_device* dev = pci->devices[i];
    int path_size = strlen(PCI_PATH) + strlen(dev->path) + 3;

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
  }
}

// Filter the input in order to get only those PCI devices which 
// we are interested in (i.e., which vendor is NVIDIA), and return
// the filtered result.
struct pci_devices * filter_pci_devices(struct pci_devices * pci) {
  int * devices_to_get = emalloc(sizeof(int) * pci->num_devices);
  int dev_ptr = 0;

  for (int i=0; i < pci->num_devices; i++) {
    if (pci->devices[i]->vendor_id == PCI_VENDOR_NVIDIA) {
      devices_to_get[dev_ptr] = i;
      dev_ptr++;
    }
  }

  struct pci_devices * pci_filtered = emalloc(sizeof(struct pci_devices));
  pci_filtered->num_devices = dev_ptr;
  if (pci_filtered->num_devices == 0)
    pci_filtered->devices = NULL;
  else
    pci_filtered->devices = emalloc(sizeof(struct pci_device) * pci_filtered->num_devices);  

  for (int i=0; i < dev_ptr; i++) {
    pci_filtered->devices[i] = pci->devices[devices_to_get[i]];
  }

  return pci_filtered;
}

struct pci_devices * get_pci_devices(void) {
  struct pci_devices * pci = get_pci_paths();

  if (pci == NULL)
    return false;

  populate_pci_devices(pci);

  struct pci_devices * filtered = filter_pci_devices(pci);  

  return filtered;
}