#include <bus/pci.h>
#include <list.h>
#include <memory.h>
#include <mutex.h>

static list_t* device_list;
static mutex_t device_list_lock = MUTEX_INITIALIZER;

static int pci_is_present(uint8_t bus, uint8_t slot, uint8_t func)
{
  pci_dev_t dev = {
    .bus_no = bus,
    .slot_no = slot,
    .function_no = func
  };

  if (pci_read16(&dev, 0x00) == 0xffff)
    return false;
  return true;
}

void pci_init()
{
  debug(PCIBUS, "enumerating PCI bus\n");

  mutex_lock(&device_list_lock);
  device_list = list_init();

  for (int bus = 0; bus < 256; bus++)
  {
    for (int slot = 0; slot < 32; slot++)
    {
      if (pci_is_present(bus, slot, 0))
      {
        for (int fnc = 0; fnc < 8; fnc++)
        {
          if (pci_is_present(bus, slot, fnc))
          {
            pci_dev_t* dev = kmalloc(sizeof(pci_dev_t));
            dev->bus_no = bus;
            dev->slot_no = slot;
            dev->function_no = fnc;

            dev->id.vendor = pci_read16(dev, PCI_VENDOR_ID);
            dev->id.device = pci_read16(dev, PCI_DEVICE_ID);
            dev->revision  = pci_read8(dev, PCI_REVISION);
            dev->prog_if   = pci_read8(dev, PCI_PROG_IF);
            dev->subclass  = pci_read8(dev, PCI_SUBCLASS);
            dev->class     = pci_read8(dev, PCI_CLASS);

            debug(PCIBUS, "  %x:%x -> class=%zd, subclass=%zd\n",
                  dev->id.vendor, dev->id.device,
                  dev->class, dev->subclass);

            list_add(device_list, dev);
          }
        }
      }
    }
  }

  mutex_unlock(&device_list_lock);
}

void pci_register_driver(const pci_driver_t* drv_info)
{
  // search device list
  debug(PCIBUS, "register driver %s\n", drv_info->name);
  mutex_lock(&device_list_lock);
  size_t size = list_size(device_list);
  for (int i = 0; i < size; i++)
  {
    const pci_idpair_t* ids = drv_info->devices;
    pci_dev_t* dev = list_get(device_list, i);
    while (ids->device != 0 && ids->vendor != 0)
    {
      if (ids->device == dev->id.device
          && ids->vendor == dev->id.vendor)
      {
        mutex_unlock(&device_list_lock);
        drv_info->probe(dev);
        return;
      }

      ids++;
    }
  }
  mutex_unlock(&device_list_lock);
}

uint16_t pci_read16(pci_dev_t *dev, uint8_t offset)
{
  uint32_t data = pci_read32(dev, offset);
  return ((data >> ((offset & 2) * 8)) & 0xffff);
}

uint8_t pci_read8(pci_dev_t *dev, uint8_t offset)
{
  uint32_t data = pci_read32(dev, offset);
  return ((data >> ((offset & 3) * 8)) & 0xff);
}

uint32_t pci_get_bar(pci_dev_t* device, uint8_t bar)
{
  return pci_read32(device, PCI_BAR0 + bar * 4);
}
