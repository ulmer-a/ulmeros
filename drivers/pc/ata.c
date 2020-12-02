/*
 * ULMER Operating System
 *
 * ATA PCI IDE controller driver
 * this driver can control one or more IDE controllers
 * present on the system's PCI bus to be able to read and
 * write sectors to and from IDE/SATA hard disk drives.
 *
 * this driver currently supports the following devices:
 *  - Intel 82371AB/EB/MB PIIX4 IDE Controller (not tested)
 *  - Intel 82371SB PIIX3 IDE Controller [Natoma/Triton II]
 *
 * Copyright (C) 2018-2020
 * Written by Alexander Ulmer <ulmer@student.tugraz.at>
 *
 * Some of the driver is based on code that can be found at
 * https://wiki.osdev.org/PCI_IDE_Controller (accessed Dec 2, 2020)
 */

#include <types.h>
#include <bus/pci.h>
#include <memory.h>
#include <fs/blockdev.h>
#include <list.h>
#include <amd64/ports.h>
#include <mutex.h>
#include <errno.h>

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

/* the driver's major number. it is assigned
 * by the kernel when the driver registers itself. */
static size_t ata_major;

/* list of IDE controllers managed by this driver */
static list_t* controller_list;
static mutex_t controller_list_lock = MUTEX_INITIALIZER;

/**
 * @brief ide_write write to a controller's configuration register
 * @param controller the controller managing the device
 * @param channel 0=primary 1=secondary
 * @param reg register to be written. use constants above.
 * @param data value to be written
 */
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

/**
 * @brief ide_write read from a controller's configuration register
 * @param controller the controller managing the device
 * @param channel 0=primary 1=secondary
 * @param reg register to be written. use constants above.
 * @return dava value
 */
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

/**
 * @brief msleep sleep n milliseconds
 * this method should be improved and moved to the kernel!
 * @param ms
 */
static void msleep(size_t ms)
{
  size_t waitfor = ms * 10000000;
  while (waitfor--);
}

/**
 * @brief ata_read
 * @param minor
 * @param buf
 * @param count
 * @param lba
 * @return
 */
static ssize_t ata_read(size_t minor, char* buf,
                        size_t count, size_t lba)
{
  return -ENOSYS;
}

/**
 * @brief ata_write
 * @param minor
 * @param buf
 * @param count
 * @param lba
 * @return
 */
static ssize_t ata_write(size_t minor, char* buf,
                         size_t count, size_t lba)
{
  return -ENOSYS;
}

/**
 * @brief ide_identify perform an identification operation
 * on the sepecified drive connected to a given controller
 * @param controller the IDE controller
 * @param channel 0=Primary, 1=Secondary
 * @param drive 0=Master, 1=Slave
 */
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

  /* poll until success */
  uint8_t status;
  while (1)
  {
    status = ide_read(controller, channel, ATA_REG_STATUS);

    if (status & ATA_SR_ERR)
    {
      /* device is not ATA! maybe perform some further
       * detection. probably the device is ATAPI/SCSI */
      debug(ATADISK, "%s-%s: SCSI is unimplemented\n",
            channel == 0 ? "Primary" : "Secondary",
            drive == 0 ? "Master" : "Slave");
      return;
    }

    if ((!(status & ATA_SR_BSY)) && (status & ATA_SR_DRQ))
    {
      /* everything is alright! */
      break;
    }
  }

  /* read the identification space of the specified
   * device. this will return detailed info about the device. */
  uint8_t idspace[512];
  if ((ide_read(controller, channel,
                ATA_REG_STATUS) & ATA_SR_ERR) == 0)
  {
    repinsw(controller->ide_channels[channel].base
            + ATA_REG_DATA, (uint16_t*)idspace, 256);
  }

  /* complete the missing data in the ide_device[] entry. this
   * includes information like serial number, model description,
   * command sets and the capacity */
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

/* block device driver description structure. this
 * will tell the block device manager what kind of
 * operations this driver supports and how to call them. */
static bd_driver_t ata_bd_driver = {
  .name = "pc_ata_pci",
  .file_prefix = "hd",
  .fops = {
    .read = ata_read,
    .write = ata_write
  }
};

static int ata_probe(pci_dev_t* device)
{
  /* some asserting checks */
  if (device->class != 1 && device->subclass != 1)
    return false;

  debug(ATADISK, "%x:%x: PCI IDE PATA/SATA Hard Drive controller\n",
        device->id.vendor, device->id.device);

  /* get the IO space addresses from the PCI
   * configuration space base address registers. */
  uint32_t bar0 = pci_get_bar(device, 0); // Primary Channel IO Base
  uint32_t bar1 = pci_get_bar(device, 1); // Primary Channel Control Base
  uint32_t bar2 = pci_get_bar(device, 2); // Secondary Channel IO Base
  uint32_t bar3 = pci_get_bar(device, 3); // Secondary Channel Control Base
  uint32_t bar4 = pci_get_bar(device, 4); // Secondary Channel Control Base

  if (bar0 == 0x0 || bar0 == 0x1) bar0 = 0x1f0;
  if (bar1 == 0x0 || bar1 == 0x1) bar1 = 0x3f6;
  if (bar2 == 0x0 || bar2 == 0x1) bar2 = 0x170;
  if (bar3 == 0x0 || bar3 == 0x1) bar3 = 0x376;

  /* initialize the IDE channels of the controller
   * structure describing this device. */
  pci_ide_dev_t* controller = kmalloc(sizeof(pci_ide_dev_t));
  controller->ide_channels[ATA_PRIMARY].base =        (bar0 & 0xfffffffc);
  controller->ide_channels[ATA_PRIMARY].ctrl =        (bar1 & 0xfffffffc);
  controller->ide_channels[ATA_PRIMARY].busmaster =   (bar4 & 0xfffffffc);
  controller->ide_channels[ATA_SECONDARY].base =      (bar2 & 0xfffffffc);
  controller->ide_channels[ATA_SECONDARY].ctrl =      (bar3 & 0xfffffffc);
  controller->ide_channels[ATA_SECONDARY].busmaster = (bar4 & 0xfffffffc) + 8;

  /* disable IRQ's on both channels */
  ide_write(controller, ATA_PRIMARY, ATA_REG_CONTROL, 2);
  ide_write(controller, ATA_SECONDARY, ATA_REG_CONTROL, 2);

  /* identifiy each of the drives. this checks their type,
   * if they are present and obtains some basic parameters,
   * like capacity, model and serial numbers, ... */
  ide_identify(controller, ATA_PRIMARY, ATA_MASTER);
  ide_identify(controller, ATA_PRIMARY, ATA_SLAVE);
  ide_identify(controller, ATA_SECONDARY, ATA_MASTER);
  ide_identify(controller, ATA_SECONDARY, ATA_SLAVE);

  /* add the IDE controller to the list of controllers
   * managed by this device. since there can be multiple,
   * we have to synchronize this properly. */
  mutex_lock(&controller_list_lock);
  size_t minors = list_size(controller_list) * 4;
  list_add(controller_list, controller);
  mutex_unlock(&controller_list_lock);

  /* iterate through the devices that are present and
   * print some descriptive information to the kernel
   * output. */
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

    /* finally, register all the devices as
     * block devices, so they can be referenced. */
    bd_t* disk = kmalloc(sizeof(bd_t));
    disk->driver = &ata_bd_driver;
    disk->minor = minors + i;
    disk->capacity = dev->sectors;
    bd_register(disk);
  }

  return true;
}

/* PCI driver description structure. this will tell
 * the kernel which devices this driver can handle. */
static const pci_idpair_t ata_pci_ids[] = {
  { 0x8086, 0x7111 }, // Intel 82371AB/EB/MB PIIX4 IDE Controller (not tested)
  { 0x8086, 0x7010 }, // Intel 82371SB PIIX3 IDE Controller [Natoma/Triton II]
  { 0, 0 }
};

/* PCI driver description structure function map */
static const pci_driver_t ata_pci_driver = {
  .devices = ata_pci_ids,
  .probe = ata_probe
};

void ata_init()
{
  debug(ATADISK, "loading SATA/PATA disk driver\n");

  /* initialize the list of IDE controllers handled
   * by this driver. */
  controller_list = list_init();

  /* obtain a global major-number by registering as
   * a block device driver. */
  ata_major = bd_register_driver(&ata_bd_driver);

  /* register as a PCI device driver. this will result
   * in a call to ata_probe() in the case that a device
   * that can be handled by this driver is present. this
   * should be the last thing to be done in ata_init(). */
  pci_register_driver(&ata_pci_driver);
}
