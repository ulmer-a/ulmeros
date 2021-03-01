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
 * Copyright (C) 2018-2021
 * Written by Alexander Ulmer <ulmer@student.tugraz.at>
 */

#include <util/types.h>
#include <util/list.h>
#include <util/mutex.h>
#include <util/string.h>
#include <bus/pci.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <fs/blockdev.h>
#include <x86/ports.h>
#include <errno.h>
#include <debug.h>
#include <sched/task.h>
#include <sched/interrupt.h>

// Status
#define ATA_SR_BSY      0x80    // Busy
#define ATA_SR_DRDY     0x40    // Drive ready
#define ATA_SR_DF       0x20    // Drive write fault
#define ATA_SR_DSC      0x10    // Drive seek complete
#define ATA_SR_DRQ      0x08    // Data request ready
#define ATA_SR_CORR     0x04    // Corrected data
#define ATA_SR_IDX      0x02    // Index
#define ATA_SR_ERR      0x01    // Error

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
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
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
#define      ATA_READ         0x00
#define      ATA_WRITE        0x01

// ----------- PCI busmastering DMA ---------------------------
#define DMA_CMD                   0x00
#define DMA_STAT                  0x02
#define DMA_ADDR                  0x04

#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_DMA         0xca
#define ATA_CMD_WRITE_DMA_EXT     0x35

#define DMA_CMD_START     BIT(0) // BIT(0) = 1
#define DMA_CMD_STOP      0      // BIT(0) = 0
#define DMA_CMD_READ      BIT(3) // BIT(3) = 1
#define DMA_CMD_WRITE     0      // BIT(3) = 0

#define DMA_STAT_ACTIVE   BIT(0)  // DMA is active
#define DMA_STAT_FAIL     BIT(1)  // DMA failed
#define DMA_STAT_IRQ      BIT(2)  // IRQ raised
#define DMA_STAT_MA_CAP   BIT(5)  // Master is ready for DMA
#define DMA_STAT_SL_CAP   BIT(6)  // Slave is ready for DMA

#define PRD_PAGES   ((1024*64)/4096)  // 64K / PAGE_SIZE
#define BLOCKS_PER_PRD    (((1024*64)/512)-1)
#define BLOCK_SIZE 512

typedef struct
{
  uint64_t buffer     : 32;
  uint64_t bytes      : 16;
  uint64_t last_entry : 1;
  uint64_t reserved   : 15;
} __attribute__((packed)) region_desc_t;

// ------------- driver handling structures --------------------
typedef struct
{
  region_desc_t* prdt;

  size_t data_ready;
  uint8_t irq_status;
  task_t* signal_task;

  uint16_t base;
  uint16_t ctrl;
  uint16_t busmaster;
  uint16_t nIEN;
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
  char model[41];     // model string
} ide_dev_t;

typedef struct
{
  size_t minor_base;
  mutex_t transfer_lock;
  ide_channel_t ide_channels[2];
  ide_dev_t ide_devices[4];
} pci_ide_dev_t;

/* the driver's major number. it is assigned
 * by the kernel when the driver registers itself. */
static size_t ata_major;
static size_t minor_counter = 0;

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

static void ata_irq(void* driver_data)
{
  ide_channel_t* channel = driver_data;
  size_t busmaster_base = channel->busmaster;

  // check if the IRQ bit is set in the busmaster
  // status register, then clear it
  uint8_t status = inb(busmaster_base + DMA_STAT);
  outb(busmaster_base + DMA_STAT, 4);

  if (status & DMA_STAT_IRQ)
  {
    // if the device generated an IRQ, wake up the
    // task that is waiting for it.
    channel->data_ready = true;
    channel->irq_status = status;
    irq_signal(channel->signal_task);
  }

}

/**
 * @brief ata_read
 * @param minor
 * @param buf
 * @param count
 * @param lba
 * @return
 */
static ssize_t ata_read(void* drv, size_t minor,
                        char* buf, size_t count, uint64_t lba)
{
  pci_ide_dev_t* controller = drv;
  if (controller == NULL)
    return -ENODEV;

  mutex_lock(&controller->transfer_lock);

  uint8_t ch_no = ATA_PRIMARY;
  if (minor % 4 >= 2)
    ch_no = ATA_SECONDARY;
  uint8_t drive = minor % 2;
  ide_channel_t* channel = &(controller
      ->ide_channels[ch_no]);

  char* current_buffer = buf;
  uint64_t current_lba = lba;
  size_t blocks_read = 0;
  size_t blocks_remaining = count;
  while (blocks_remaining > 0)
  {
    size_t tr_size = blocks_remaining;
    if (tr_size > BLOCKS_PER_PRD)
      tr_size = BLOCKS_PER_PRD;

    debug(ATADISK, "ata-dma: read(): count=%zd, lba=%zd\n",
          tr_size, current_lba);

    // prepare the physical region descriptor
    channel->prdt->bytes
        = tr_size * 512;

    // reset the interrupt status
    channel->data_ready = 0;
    channel->signal_task = current_task;

    // set the DMA data direction to READ
    outb(channel->busmaster + DMA_CMD, DMA_CMD_READ|DMA_CMD_STOP);

    // clear FAIL and IRQ bit in DMA status register
    uint8_t dma_stat = inb(channel->busmaster + DMA_STAT);
    dma_stat &= ~(DMA_STAT_FAIL | DMA_STAT_IRQ);
    outb(channel->busmaster + DMA_STAT, dma_stat);

    const size_t iobase = channel->base;

    outb(iobase + ATA_REG_HDDEVSEL, 0xe0 | (drive << 4));
    outb(iobase + ATA_REG_SECCOUNT0, tr_size >> 8);
    outb(iobase + ATA_REG_LBA0, (current_lba >> 24) & 0xff);
    outb(iobase + ATA_REG_LBA1, (current_lba >> 32) & 0xff);
    outb(iobase + ATA_REG_LBA2, (current_lba >> 40) & 0xff);
    outb(iobase + ATA_REG_SECCOUNT0, tr_size & 0xff);
    outb(iobase + ATA_REG_LBA0, (current_lba) & 0xff);
    outb(iobase + ATA_REG_LBA1, (current_lba >>  8) & 0xff);
    outb(iobase + ATA_REG_LBA2, (current_lba >> 16) & 0xff);

    /* send DMA_READ_EXT command */
    while (inb(iobase + ATA_REG_STATUS) & ATA_SR_BSY);
    outb(iobase + ATA_REG_COMMAND, ATA_CMD_READ_DMA_EXT);
    while (inb(iobase + ATA_REG_STATUS) & ATA_SR_BSY);

    /* set the DMA START bit. this will start the
     * transfer to memory. */
    outb(channel->busmaster + DMA_CMD, DMA_CMD_READ|DMA_CMD_START);
    debug(ATADISK, "ata-dma: started transfer\n");

    /* while the device transfers data to memory, this
     * thread can go to sleep. */
    //irq_wait_until(&channel->data_ready, true);
    //assert(channel->data_ready, "irq not ready!");
    debug(ATADISK, "ata-dma: transfer complete\n");

    /* the transfer completed, so stop DMA. */
    outb(channel->busmaster + DMA_CMD, DMA_CMD_READ|DMA_CMD_STOP);

    uint8_t status = inb(channel->base + ATA_REG_STATUS);
    if ((status & ATA_SR_ERR) || (status & ATA_SR_DF))
    {
      mutex_unlock(&controller->transfer_lock);
      debug(ATADISK, "ata-dma: read() failed\n");
      return blocks_read;
    }

    /* copy data from DMA buffer to the actual buffer */
    void* data_ptr = (char*)channel->prdt + 512;
    memcpy(current_buffer, data_ptr, tr_size * 512);

    /* update our control variables */
    blocks_remaining -= tr_size;
    blocks_read += tr_size;
    current_lba += tr_size;
    current_buffer += (tr_size * 512);
  }

  mutex_unlock(&controller->transfer_lock);
  return blocks_read;
}

/**
 * @brief ata_write
 * @param minor
 * @param buf
 * @param count
 * @param lba
 * @return
 */
static ssize_t ata_write(void* drv, size_t minor,
                         char* buf, size_t count, uint64_t lba)
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

static const char* ata_get_prefix(void* drv)
{
  (void)drv;
  return NULL;
}

/* block device driver description structure. this
 * will tell the block device manager what kind of
 * operations this driver supports and how to call them. */
static bd_driver_t ata_bd_driver = {
  .name = "ata_pci",
  .prefix = "hdd",
  .bd_ops = {
    .readblk = ata_read,
    .writeblk = ata_write,
    .get_prefix = ata_get_prefix
  }
};

static void ata_setup_dma(pci_ide_dev_t* controller, uint8_t channel)
{
  /* allocate a DMA buffer that doesn't cross 64K boundaries
   * and resides in physical memory below 4GB */
  void* phys_buffer = alloc_dma_region();
  controller->ide_channels[channel].prdt =
      (phys_buffer);
  controller->ide_channels[channel].prdt->buffer =
      (size_t)phys_buffer + BLOCK_SIZE;
  controller->ide_channels[channel].prdt->last_entry = 1;

  /* store the physical address of the PRDT in the
   * corresponding BusMaster ADDR register */
  size_t busmaster_reg = controller->ide_channels[channel].busmaster;
  outl(busmaster_reg + DMA_ADDR, (size_t)phys_buffer);
}

static void* ata_probe(pci_dev_t* device)
{
  //if (device->class != 1 && device->subclass != 1)
  //  return false;

  debug(ATADISK, "%x:%x: PCI IDE PATA/SATA Hard Drive controller\n",
        (uint32_t)device->id.vendor, (uint32_t)device->id.device);

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

  /* enable PCI busmastering. this is very important,
   * because otherwise the device could not write to
   * main memory by itself. */
  pci_enable_busmaster(device);

  /* initialize the IDE channels of the controller
   * structure describing this device. */
  pci_ide_dev_t* controller = kmalloc(sizeof(pci_ide_dev_t));
  controller->minor_base = atomic_add(&minor_counter, 4);
  mutex_init(&controller->transfer_lock);
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

  /* setup PCI busmastering DMA */
  if(controller->ide_devices[0].present ||
     controller->ide_devices[1].present)
  {
    irq_subscribe(IRQ_ATA_PRIM, "ata-hdd", ata_irq,
                 &(controller->ide_channels[0]));
    ata_setup_dma(controller, ATA_PRIMARY);
  }

  if(controller->ide_devices[2].present ||
     controller->ide_devices[3].present)
  {
    irq_subscribe(IRQ_ATA_SEC, "ata-hdd", ata_irq,
                 &(controller->ide_channels[1]));
    ata_setup_dma(controller, ATA_SECONDARY);
  }

  /* enable IRQ's on both channels */
  ide_write(controller, ATA_PRIMARY, ATA_REG_CONTROL, 0);
  ide_write(controller, ATA_SECONDARY, ATA_REG_CONTROL, 0);

  /* iterate through the devices that are present and
   * print some descriptive information to the kernel
   * output. */
  for (size_t i = 0; i < 3; i++)
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
    disk->data = controller;
    disk->driver = &ata_bd_driver;
    disk->minor = controller->minor_base + i;
    disk->capacity = dev->sectors;
    sprintf(disk->name, "hdd%zu", i);
    bd_register(disk);
  }

  return controller;
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
  .name = "ata_pci",
  .devices = ata_pci_ids,
  .probe = ata_probe
};

void pc_ata_init()
{
  /* obtain a global major-number by registering as
   * a block device driver. */
  ata_major = bd_register_driver(&ata_bd_driver);

  /* register as a PCI device driver. this will result
   * in a call to ata_probe() in the case that a device
   * that can be handled by this driver is present. this
   * should be the last thing to be done in ata_init(). */
  pci_register_driver(&ata_pci_driver);
}
