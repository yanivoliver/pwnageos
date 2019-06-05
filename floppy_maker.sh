#!/bin/bash

set -e

rm -f bin/boot.img
mkfs.msdos -C bin/boot.img -F12 1440
mkdir -p /tmp/pwnageos
sudo mount bin/boot.img /tmp/pwnageos/
sudo cp bin/*.bin /tmp/pwnageos/
sudo umount /tmp/pwnageos/
dd conv=notrunc if=bin/bootloader of=bin/boot.img bs=512 count=1
