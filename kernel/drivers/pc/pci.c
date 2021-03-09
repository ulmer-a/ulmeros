/*
 * UlmerOS PCI configuration space access
 * for i386-i786 and x86_64 architectures
 * Copyright (C) 2021 Alexander Ulmer
 *
 * pci_read32() and pci_write32() provide thread-safe
 * access to the PCI configuration space of x86 PCs
 */

#include <x86/ports.h>
#include <bus/pci.h>
#include <sched/mutex.h>

#define CONFIG_ADDRESS  0xcf8
#define CONFIG_DATA     0xcfc

static spin_t pci_spinlock = SPIN_INITIALIZER;

uint32_t pci_read32(pci_dev_t *dev, uint8_t offset)
{
  uint32_t address;
  uint32_t bus = dev->bus;
  uint32_t slot = dev->slot;
  uint32_t fnc = dev->func;

  address = (bus << 16)
          | (slot << 11)
          | (fnc << 8)
          | (offset & 0xfc)
          | BIT(31);

  spin_lock(&pci_spinlock);
  outl(CONFIG_ADDRESS, address);
  uint32_t ret = inl(CONFIG_DATA);
  spin_unlock(&pci_spinlock);
  return ret;
}

void pci_write32(pci_dev_t *dev, uint8_t offset, uint32_t value)
{
  uint32_t address;
  uint32_t bus = dev->bus;
  uint32_t slot = dev->slot;
  uint32_t fnc = dev->func;

  address = (bus << 16)
          | (slot << 11)
          | (fnc << 8)
          | (offset & 0xfc)
          | BIT(31);

  spin_lock(&pci_spinlock);
  outl(CONFIG_ADDRESS, address);
  outl(CONFIG_DATA, value);
  spin_unlock(&pci_spinlock);
}
