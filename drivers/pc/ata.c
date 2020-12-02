#include <types.h>
#include <bus/pci.h>
#include <memory.h>
#include "ports.h"

// Status
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Error
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

// Commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

// Identification space
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

// task file 8 ports per IDE channel
// Numbers are offsets from BAR0/BAR2
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01
#define      ATA_MASTER       0x00
#define      ATA_SLAVE        0x01

// Directions
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

typedef struct
{
  uint16_t base;
  uint16_t ctrl;
  uint16_t busmaster;
  uint8_t  nIEN;
} ide_channel_t;

typedef struct
{
  uint8_t present;    // 1=present
  uint8_t channel;    // 0=primary, 1=secondary
  uint8_t drive;      // 0=master, 1=slave
  uint16_t type;      // 0=ATA, 1=ATAPI
  uint16_t signat;    // drive signature
  uint16_t capa;      // drive capabilities
  uint32_t cmd_sets;  // supported command sets
  uint32_t sectors;   // size in sectors
  char model[41]; // model string
} ide_dev_t;

typedef struct
{
  ide_channel_t ide_channels[2];
  ide_dev_t ide_devices[4];
} pci_ide_dev_t;



static void ide_write(pci_ide_dev_t* controller, uint8_t channel,
                      uint8_t reg, uint8_t data)
{
  if (reg > 0x07 && reg < 0x0c)
  {
    ide_write(controller, channel, ATA_REG_CONTROL,
              0x80 | controller->ide_channels[channel].nIEN);
  }

  if (reg < 0x08)
    outb(controller->ide_channels[channel].base + reg - 0x00, data);
  else if (reg < 0x0c)
    outb(controller->ide_channels[channel].base + reg - 0x06, data);
  else if (reg < 0x0e)
    outb(controller->ide_channels[channel].ctrl + reg - 0x0a, data);
  else if (reg < 0x16)
    outb(controller->ide_channels[channel].busmaster + reg - 0x0e, data);

  if (reg > 0x07 && reg < 0x0c)
  {
    ide_write(controller, channel, ATA_REG_CONTROL,
              controller->ide_channels[channel].nIEN);
  }
}

static uint8_t ide_read(pci_ide_dev_t* controller, uint8_t channel,
                        uint8_t reg)
{
  uint8_t result;
  if (reg > 0x07 && reg < 0x0c)
  {
    ide_write(controller, channel, ATA_REG_CONTROL,
              0x80 | controller->ide_channels[channel].nIEN);
  }

  if (reg < 0x08)
    result = inb(controller->ide_channels[channel].base + reg - 0x00);
  else if (reg < 0x0c)
    result = inb(controller->ide_channels[channel].base + reg - 0x06);
  else if (reg < 0x0e)
    result = inb(controller->ide_channels[channel].ctrl + reg - 0x0a);
  else if (reg < 0x16)
    result = inb(controller->ide_channels[channel].busmaster + reg - 0x0e);

  if (reg > 0x07 && reg < 0x0c)
  {
    ide_write(controller, channel, ATA_REG_CONTROL,
              controller->ide_channels[channel].nIEN);
  }
  return result;
}

static void msleep(size_t ms)
{
  size_t waitfor = ms * 10000000;
  while (waitfor--);
}

static void ide_identify(pci_ide_dev_t* controller, uint8_t channel, uint8_t drive)
{
  uint8_t drv_id = channel * 2 + drive;
  ide_dev_t* ide_device = &(controller->ide_devices[drv_id]);

  ide_device->present = 0; // assume it's not present

  // select drive
  ide_write(controller, channel, ATA_REG_HDDEVSEL, 0xA0 | (drive << 4));
  msleep(1);

  // send ATA identify command
  ide_write(controller, channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
  msleep(1);

  // check status, if it is zero, there's no device present
  if (ide_read(controller, channel, ATA_REG_STATUS) == 0)
    return;

  uint8_t status;
  while (1)
  {
    status = ide_read(controller, channel, ATA_REG_STATUS);

    if (status & ATA_SR_ERR)
    {
      // device is not ATA!
      debug(ATADISK, "%s-%s: SCSI is unimplemented\n",
            channel == 0 ? "Primary" : "Secondary",
            drive == 0 ? "Master" : "Slave");
      return;
    }

    // check if everything is alright
    if ((!(status & ATA_SR_BSY)) && (status & ATA_SR_DRQ))
      break;
  }

  // read identification space
  uint8_t idspace[512];
  if ((ide_read(controller, channel, ATA_REG_STATUS) & ATA_SR_ERR) == 0)
  {
    repinsw(controller->ide_channels[channel].base + ATA_REG_DATA, (uint16_t*)idspace, 256);
  }

  ide_device[drv_id].present  = 1;
  ide_device[drv_id].channel  = channel;
  ide_device[drv_id].drive    = drive;
  ide_device[drv_id].signat   = *((uint16_t*)(idspace + ATA_IDENT_DEVICETYPE));
  ide_device[drv_id].capa     = *((uint16_t*)(idspace + ATA_IDENT_CAPABILITIES));
  ide_device[drv_id].cmd_sets = *((uint32_t*)(idspace + ATA_IDENT_COMMANDSETS));

  if (ide_device[drv_id].cmd_sets & BIT(26))
   ide_device[drv_id].sectors = *((uint32_t*)(idspace + ATA_IDENT_MAX_LBA_EXT));
  else
   ide_device[drv_id].sectors = *((uint32_t*)(idspace + ATA_IDENT_MAX_LBA));

  for (int i = 0; i < 40; i += 2)
  {
   ide_device[drv_id].model[i] = idspace[ATA_IDENT_MODEL + i + 1];
   ide_device[drv_id].model[i + 1] = idspace[ATA_IDENT_MODEL + i];
  }
  for (int i = 40; i >= 0; i--)
  {
    if (ide_device[drv_id].model[i] != ' '
        && ide_device[drv_id].model[i] != 0)
      break;

    if (ide_device[drv_id].model[i] == ' ')
      ide_device[drv_id].model[i] = 0;
  }
  ide_device[drv_id].model[40] = 0;
}

static int ata_probe(pci_dev_t* device)
{
  if (device->class != 1 && device->subclass != 1)
    return false;

  debug(ATADISK, "%x:%x: PCI IDE PATA/SATA Hard Drive controller\n",
        device->id.vendor, device->id.device);

  uint32_t bar0 = pci_get_bar(device, 0); // Primary Channel IO Base
  uint32_t bar1 = pci_get_bar(device, 1); // Primary Channel Control Base
  uint32_t bar2 = pci_get_bar(device, 2); // Secondary Channel IO Base
  uint32_t bar3 = pci_get_bar(device, 3); // Secondary Channel Control Base
  uint32_t bar4 = pci_get_bar(device, 4); // Secondary Channel Control Base

  if (bar0 == 0x0 || bar0 == 0x1) bar0 = 0x1f0;
  if (bar1 == 0x0 || bar1 == 0x1) bar1 = 0x3f6;
  if (bar2 == 0x0 || bar2 == 0x1) bar2 = 0x170;
  if (bar3 == 0x0 || bar3 == 0x1) bar3 = 0x376;

  pci_ide_dev_t* controller = kmalloc(sizeof(pci_ide_dev_t));
  controller->ide_channels[ATA_PRIMARY].base =        (bar0 & 0xfffffffc);
  controller->ide_channels[ATA_PRIMARY].ctrl =        (bar1 & 0xfffffffc);
  controller->ide_channels[ATA_PRIMARY].busmaster =   (bar4 & 0xfffffffc);
  controller->ide_channels[ATA_SECONDARY].base =      (bar2 & 0xfffffffc);
  controller->ide_channels[ATA_SECONDARY].ctrl =      (bar3 & 0xfffffffc);
  controller->ide_channels[ATA_SECONDARY].busmaster = (bar4 & 0xfffffffc) + 8;

  // disable IRQ's on both channels
  ide_write(controller, ATA_PRIMARY, ATA_REG_CONTROL, 2);
  ide_write(controller, ATA_SECONDARY, ATA_REG_CONTROL, 2);

  // identifiy each of the drives
  ide_identify(controller, ATA_PRIMARY, ATA_MASTER);
  ide_identify(controller, ATA_PRIMARY, ATA_SLAVE);
  ide_identify(controller, ATA_SECONDARY, ATA_MASTER);
  ide_identify(controller, ATA_SECONDARY, ATA_SLAVE);

  for (int i = 0; i < 3; i++)
  {
    ide_dev_t* dev = &controller->ide_devices[i];
    if (!dev->present)
      continue;

    debug(ATADISK, "%s-%s: LBA's=%zd (%zd MB) model=\"%s\"\n",
          dev->channel == 0 ? "Primary" : "Secondary",
          dev->drive   == 0 ? "Master" : "Slave",
          dev->sectors, dev->sectors * 512 / (1024*1024),
          dev->model);
  }

  return true;
}

static const pci_idpair_t ata_pci_ids[] = {
  { 0x8086, 0x7111 }, // Intel 82371AB/EB/MB PIIX4 IDE Controller (not tested)
  { 0x8086, 0x7010 }, // Intel 82371SB PIIX3 IDE Controller [Natoma/Triton II]
  { 0, 0 }
};

static const pci_driver_t ata_pci_driver = {
  .devices = ata_pci_ids,
  .probe = ata_probe
};

void ata_init()
{
  debug(ATADISK, "setting up SATA/PATA disk driver\n");

  pci_register_driver(&ata_pci_driver);
}
