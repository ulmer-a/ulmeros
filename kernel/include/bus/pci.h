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

typedef struct
{
  uint8_t bus_no;
  uint8_t slot_no;
  uint8_t function_no;

  pci_idpair_t id;

  uint8_t revision;
  uint8_t prog_if;
  uint8_t subclass;
  uint8_t class;
} pci_dev_t;

typedef struct
{
  const char* name;
  const pci_idpair_t* devices;
  int (*probe)(pci_dev_t*);
} pci_driver_t;

/**
 * @brief pci_init
 */
void pci_init();

/**
 * @brief pci_register_driver
 * @param drv_info
 */
void pci_register_driver(const pci_driver_t* drv_info);

/**
 * @brief pci_get_bar
 * @param device
 * @param bar
 * @return
 */
uint32_t pci_get_bar(pci_dev_t* device, uint8_t bar);

/**
 * @brief pci_read32
 * @param dev
 * @param offset
 * @return
 */
uint32_t pci_read32(pci_dev_t* dev, uint8_t offset);

/**
 * @brief pci_read16
 * @param dev
 * @param offset
 * @return
 */
uint16_t pci_read16(pci_dev_t* dev, uint8_t offset);

/**
 * @brief pci_read8
 * @param dev
 * @param offset
 * @return
 */
uint8_t pci_read8(pci_dev_t* dev, uint8_t offset);

/**
 * @brief pci_write32
 * @param dev
 * @param offset
 * @param value
 */
void pci_write32(pci_dev_t* dev, uint8_t offset, uint32_t value);
