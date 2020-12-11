#! /bin/bash

srcdir=$1

# create some directories
mkdir -p img_rootfs

# create the empty disk image (20MB)
dd if=/dev/zero of=disk.img bs=8192 count=2560

# generate msdos partition table and
# ext2 partition starting on sector 2048
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk disk.img
  o # clear the in memory partition table
  n # new partition
  p # primary partition
  1 # partition number 1
    # default - start at beginning of disk 
    # default - use entire disk
  a # make the partition bootable
  p # print the in-memory partition table
  w # write the partition table
  q # and we're done
EOF

# create loopback block devices
sudo losetup /dev/loop98 disk.img
sudo losetup /dev/loop99 disk.img -o 1048576

# format ext2 filesystem
sudo mkfs.ext2 /dev/loop99
sudo mount /dev/loop99 img_rootfs

# create directories and copy files to the image
sudo mkdir -p img_rootfs/boot
sudo mkdir -p img_rootfs/boot/grub
sudo mkdir -p img_rootfs/bin
sudo mkdir -p img_rootfs/dev
sudo mkdir -p img_rootfs/lib

sudo cp "vmulmer" img_rootfs/boot/vmulmer
sudo cp "${srcdir}/tools/grub.cfg" img_rootfs/boot/grub/grub.cfg
sudo cp userspace/progs/*.bin img_rootfs/bin/

sudo grub-install --root-directory=img_rootfs \
    --boot-directory=img_rootfs/boot \
    --no-floppy \
    --target=i386-pc \
    --modules="normal part_msdos ext2 multiboot" /dev/loop98
    
ls -la img_rootfs
ls -la img_rootfs/bin
ls -la img_rootfs/boot
ls -la img_rootfs/boot/grub

sudo umount img_rootfs
sudo losetup -d /dev/loop99
sudo losetup -d /dev/loop98
