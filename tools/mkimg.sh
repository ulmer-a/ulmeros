#! /bin/bash

if [ -z "$1" ]
then
	wd="."
else
	wd="$1"
fi

old_wd=$(pwd)
cd $wd

# create some directories
mkdir -p hddimg
mkdir -p hddimg/rootfs

# create the empty disk image (20MB)
dd if=/dev/zero of=hddimg/disk.img bs=8192 count=2560

# generate msdos partition table and
# ext2 partition starting on sector 2048
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk hddimg/disk.img
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
sudo losetup /dev/loop98 hddimg/disk.img
sudo losetup /dev/loop99 hddimg/disk.img -o 1048576

# format ext2 filesystem
sudo mkfs.ext2 /dev/loop99
sudo mount /dev/loop99 hddimg/rootfs

# create directories and copy files to the image
sudo mkdir -p hddimg/rootfs/boot
sudo mkdir -p hddimg/rootfs/boot/grub
sudo mkdir -p hddimg/rootfs/bin
sudo mkdir -p hddimg/rootfs/dev
sudo mkdir -p hddimg/rootfs/lib

sudo cp vmulmer hddimg/rootfs/boot/vmulmer
sudo cp tools/grub.cfg hddimg/rootfs/boot/grub/grub.cfg

sudo grub-install --root-directory=hddimg/rootfs \
    --boot-directory=hddimg/rootfs/boot \
    --no-floppy \
    --target=i386-pc \
    --modules="normal part_msdos ext2 multiboot" /dev/loop98
    
ls -la hddimg/rootfs
ls -la hddimg/rootfs/boot
ls -la hddimg/rootfs/boot/grub

sudo umount hddimg/rootfs
sudo losetup -d /dev/loop99
sudo losetup -d /dev/loop98

cd $old_wd
