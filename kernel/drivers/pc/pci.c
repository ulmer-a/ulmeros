#include <x86/ports.h>
#include <bus/pci.h>

#define CONFIG_ADDRESS  0xcf8
#define CONFIG_DATA     0xcfc

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

  outl(CONFIG_ADDRESS, address);
  return inl(CONFIG_DATA);
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

  outl(CONFIG_ADDRESS, address);
  outl(CONFIG_DATA, value);
}
