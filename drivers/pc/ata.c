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
#include <mm/memory.h>
#include <x86/ports.h>
#include <bus/pci.h>
#include <fs/blockdev.h>
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
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

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

#define PRD_PAGES         ((1024*64)/4096)    // 64K / PAGE_SIZE
#define BLOCKS_PER_PRD    (((1024*64)/512)-1)

static list_t controller_list;
static mutex_t controller_list_lock;
static size_t ata_major;

static ssize_t ata_read(void* drv, char* buffer, size_t count, size_t lba)
{
  return -ENOSYS;
}

static ssize_t ata_write(void* drv, char* buffer, size_t count, size_t lba)
{
  return -ENOSYS;
}

/* block device driver description structure. this
 * will tell the block device manager what kind of
 * operations this driver supports and how to call them. */
static bd_driver_t ata_bd_driver = {
  .name = "ata_pci",
  .file_prefix = "hdd",
  .bd_ops = {
    .readblk = ata_read,
    .writeblk = ata_write
  }
};

/* PCI driver description structure. this will tell
 * the kernel which devices this driver can handle. */
static const pci_idpair_t ata_pci_ids[] = {
  { 0x8086, 0x7111 }, // Intel 82371AB/EB/MB PIIX4 IDE Controller (not tested)
  { 0x8086, 0x7010 }, // Intel 82371SB PIIX3 IDE Controller [Natoma/Triton II]
  { 0, 0 }
};

/* PCI driver description structure function map */
/*static const pci_driver_t ata_pci_driver = {
  .name = "ata_pci",
  .devices = ata_pci_ids,
  .probe = ata_probe
};*/

void pc_ata_init()
{
  /* initialize the list of IDE controllers handled
   * by this driver. */
  list_init(&controller_list);
  mutex_init(&controller_list_lock);

  /* obtain a global major-number by registering as
   * a block device driver. */
  ata_major = bd_register_driver(&ata_bd_driver);

  /* register as a PCI device driver. this will result
   * in a call to ata_probe() in the case that a device
   * that can be handled by this driver is present. this
   * should be the last thing to be done in ata_init(). */
  //pci_register_driver(&ata_pci_driver);
}
