#ifndef __PCI__
#define __PCI__

#define PCI_VENDOR_NVIDIA 0x10de
#define PCI_DEVICE_TEGRA_X1 0x0faf

struct pci_device {
  char * path;
  uint16_t vendor_id;
  uint16_t device_id;
};

struct pci_devices {
  struct pci_device ** devices;
  int num_devices;
};

struct pci_devices * get_pci_devices(void);

#endif
