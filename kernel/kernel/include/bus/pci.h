#pragma once

#include <util/types.h>

/* all PCI devices: */
#define PCI_VENDOR_ID   0x00
#define PCI_DEVICE_ID   0x02
#define PCI_COMMAND     0x04
#define PCI_STATUS      0x06
#define PCI_REVISION    0x08
#define PCI_PROG_IF     0x09
#define PCI_SUBCLASS    0x0a
#define PCI_CLASS       0x0b
#define PCI_CACHE_LINE  0x0c
#define PCI_LATENCY     0x0d
#define PCI_HEADER_TYPE 0x0e
#define PCI_BIST        0x0f
#define PCI_BAR0        0x10
#define PCI_BAR1        0x14

/* if header type == 0 */
#define PCI_BAR2        0x18
#define PCI_BAR3        0x1c
#define PCI_BAR4        0x20
#define PCI_BAR5        0x24
#define PCI_CIS_PTR     0x28
#define PCI_SS_VENDOR   0x2c
#define PCI_SS_ID       0x2e
#define PCI_EXP_ROM     0x30
#define PCI_CAP_PTR     0x34
#define PCI_INTR_LINE   0x3c
#define PCI_INTR_PIN    0x3d
#define PCI_MIN_GRANT   0x3e
#define PCI_MAX_LATENC  0x3f

typedef struct
{
  uint16_t vendor;
  uint16_t device;
} pci_idpair_t;

struct _pci_driver_struct;
typedef struct _pci_driver_struct pci_driver_t;
struct _pci_dev_struct;
typedef struct _pci_dev_struct pci_dev_t;

struct _pci_driver_struct
{
  const char* name;
  const pci_idpair_t* devices;
  void* (*probe)(pci_dev_t*);
};

struct _pci_dev_struct
{
  uint8_t bus;
  uint8_t slot;
  uint8_t func;
  pci_idpair_t id;
  const pci_driver_t* driver;
  void* driver_data;
};

/* initialize PCI and perform a bus
 * enumeration scan */
void pci_init();

/* register a driver that can handle
 * certain PCI devices. */
void pci_register_driver(const pci_driver_t *drv_info);

/* PCI configuration space read/write access, this
 * is implement by platform specific code. */
uint32_t pci_read32(pci_dev_t* dev, uint8_t offset);
void pci_write32(pci_dev_t* dev, uint8_t offset, uint32_t value);

/* convenience wrappers for configuration space access
 * other than 32bit wide */
uint8_t pci_read8(pci_dev_t* dev, uint8_t offset);
uint16_t pci_read16(pci_dev_t* dev, uint8_t offset);

uint32_t pci_get_bar(pci_dev_t* device, uint8_t bar);

void pci_enable_busmaster(pci_dev_t* device);

