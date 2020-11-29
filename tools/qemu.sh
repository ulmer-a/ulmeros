#! /bin/bash

hddimg="$1"
if [ -z "$hddimg" ]
then
	hddimg="hddimg/disk.img"
fi

qemu-system-x86_64 \
	-serial file:/dev/stdout \
	-drive format=raw,file="$hddimg" \
	-m 512 -s
