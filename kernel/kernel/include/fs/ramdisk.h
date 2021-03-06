#pragma once

/* initialize ramdisk block-device driver */
void ramdisk_init();

/* install and register a new block device */
void ramdisk_install(void* ramdisk, size_t ramdisk_size);
