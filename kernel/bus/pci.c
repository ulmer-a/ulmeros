#include <bus/pci.h>
#include <debug.h>
#include <util/list.h>
#include <util/mutex.h>
#include <mm/memory.h>

static list_t device_list;
static mutex_t device_list_lock;
static int pci_initialized = false;

static int pci_is_present(uint8_t bus, uint8_t slot, uint8_t func)
{
  pci_dev_t dev = {
    .bus = bus,
    .slot = slot,
    .func = func
  };

  if (pci_read16(&dev, 0x00) == 0xffff)
    return false;
  return true;
}

void pci_init()
{
  assert(!pci_initialized, "PCI bus already initialized");

  debug(PCIBUS, "enumerating PCI bus\n");

  list_init(&device_list);
  mutex_init(&device_list_lock);

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
            /* create instance of a pci_dev_t */
            pci_dev_t* dev = kmalloc(sizeof(pci_dev_t));
            dev->bus = bus;
            dev->slot = slot;
            dev->func = fnc;

            /* obtain vendor and device id pair information */
            dev->id.vendor = pci_read16(dev, PCI_VENDOR_ID);
            dev->id.device = pci_read16(dev, PCI_DEVICE_ID);

            uint8_t revision  = pci_read8(dev, PCI_REVISION);
            uint8_t subclass  = pci_read8(dev, PCI_SUBCLASS);
            uint8_t class     = pci_read8(dev, PCI_CLASS);

            debug(PCIBUS, "  %x:%x -> class=%u, subclass=%u, rev=%u\n",
              (uint32_t)dev->id.vendor,
              (uint32_t)dev->id.device,
              (uint32_t)class,
              (uint32_t)subclass,
              (uint32_t)revision
            );

            list_add(&device_list, dev);
          }
        }
      }
    }
  }
}

void pci_register_driver(const pci_driver_t *drv_info)
{
  mutex_lock(&device_list_lock);
  for (list_item_t* it = list_it_front(&device_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    pci_dev_t* device = list_it_get(it);
    if (device->driver != NULL)
      continue;

    for (int i = 0; i < 100; i++)
    {
      const pci_idpair_t* idpair = &drv_info->devices[i];
      if (idpair->vendor == 0 && idpair->device == 0)
        break;

      /* check if the driver's device fits the current
       * device from the list. */
      if (device->id.vendor == idpair->vendor &&
          device->id.device == idpair->device)
      {
        void* drv_data = drv_info->probe(device);
        if (drv_data)
        {
          debug(PCIBUS, "%u:%u:%u: registered driver '%s'\n",
                device->bus, device->slot, device->func, drv_info->name);
          device->driver = drv_info;
          device->driver_data = drv_data;
        }
      }
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

void pci_enable_busmaster(pci_dev_t *device)
{
  uint32_t cmd = pci_read32(device, PCI_COMMAND);
  cmd |= BIT(2);
  pci_write32(device, PCI_COMMAND, cmd);
}
