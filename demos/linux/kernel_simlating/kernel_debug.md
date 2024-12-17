# kernel_debug

## kernel

### ref

`https://www.linuxprobe.com/processof-linuxrootfsand-linuxrc.html`

### download kernel

```bash
# git clone 
git clone https://mirrors.ustc.edu.cn/linux.git/
# or download tgz
wget https://mirrors.tuna.tsinghua.edu.cn/kernel/v6.x/linux-6.12.tar.xz
```

### build and config

```bash
sudo apt install libncurses-dev flex bison libssl-dev libelf-dev -y

mkdir build; cd build;
make -C ../linux/ O=$(pwd) defconfig
make -C ../linux/ O=$(pwd) allnoconfig
make -C ../linux/ O=$(pwd) menuconfig 
make -C ../linux/ O=$(pwd) distclean # clean .config as well
make -C ../linux/ O=$(pwd) bzImage
make -C ../linux/ O=$(pwd) mrproper
make -C ../linux/ O=$(pwd) -j9
make -C ../linux/ INSTALL_PATH=$(pwd)/output O=$(pwd) install

# -->
# arch/x86/boot/bzImage
# 
```


### start up

```bash

/etc/inittab

# or

/sbin/init --> systemd

```


## busybox

### download

```bash
wget https://busybox.net/downloads/busybox-1.36.1.tar.bz2
```

### build

```bash
make help
make allyesconfig
make defconfig
make menuconfig

make install
make install-hardlinks


## ensure static build
make KBUILD_SRC=../busybox-1.36.1 -f ../busybox-1.36.1/Makefile defconfig
make KBUILD_SRC=../busybox-1.36.1 -f ../busybox-1.36.1/Makefile CONFIG_PREFIX=./output install 

/etc/inittab
::sysinit:/bin/bash
::respawn:/bin/sh

```


## rootfs

### 

```bash
mkdir  dev  etc  home  lib   mnt  proc  root   sys  tmp   var -p
chown root:root rootfs/* -R
```

### build rootfs

```bash
# create disk
qemu-img create -f raw disk.img 1G
# create partition
fdisk ./disk.img
# after partition create

# use losetup
sudo losetup -fP ./disk.img 
sudo losetup -a
sudo mkfs.ext4 /dev/loop6p1
sudo mount /dev/loop5p1 test
sudo cp -Pr rootfs/* test/
sudo umount test
sudo losetup -d /dev/loop5

# fdisk -l disk.img
Disk disk.img: 1 GiB, 1073741824 bytes, 2097152 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x1429fb50

Device     Boot Start     End Sectors  Size Id Type
disk.img1        2048 2097151 2095104 1023M 83 Linux
# make squashfs fs for root
mksquashfs rootfs rootfs.squashfs -no-xattrs -all-root
mksquashfs rootfs /dev/loop5p1 -no-xattrs -all-root -noappend
# copy fs to disk's root partition(default sda)
dd if=rootfs.squashfs of=disk.img bs=1 count=651264 seek=2048 conv=notrunc
# 
qemu-system-x86_64 -kernel ../kernel/build/output/vmlinuz-6.12.0-10553-gb86545e02e8c -m 512M -nographic -serial mon:stdio -append "root=/dev/sda1 rw console=ttyS0 init=/linuxrc" -drive file=./disk.img,format=raw -accel kvm -smp 2
```

### fs

```bash
# /etc/inittab
#
# Copyright (C) 2001 Erik Andersen <andersen@codepoet.org>
#
# Note: BusyBox init doesn't support runlevels.  The runlevels field is
# completely ignored by BusyBox init. If you want runlevels, use
# sysvinit.
#
# Format for each entry: <id>:<runlevels>:<action>:<process>
#
# id        == tty to run on, or empty for /dev/console
# runlevels == ignored
# action    == one of sysinit, respawn, askfirst, wait, and once
# process   == program to run

# Startup the system
::sysinit:/bin/mount -t proc proc /proc
::sysinit:/bin/mount -o remount,ro,noatime /
::sysinit:/bin/mount -t devtmpfs tmpfs /dev
::sysinit:/bin/mkdir -p /dev/pts
::sysinit:/bin/mkdir -p /dev/shm
::sysinit:/bin/mount -a
::sysinit:/bin/hostname -F /etc/hostname
# now run any rc scripts
::sysinit:/etc/init.d/rcS

# Put a getty on the serial port
#ttySLB4::respawn:/sbin/getty -L ttySLB4 0 vt100 # GENERIC_SERIAL
#ttyS0::respawn:/sbin/getty -L ttyS0 0 vt100 # GENERIC_SERIAL
#ttyS0::respawn:/sbin/getty -L ttyS0 115200 vt100 # GENERIC_SERIAL

ttyS0::respawn:/bin/sh # GENERIC_SERIAL

# Stuff to do for the 3-finger salute
#::ctrlaltdel:/sbin/reboot

# Stuff to do before rebooting
::shutdown:/etc/init.d/rcK
::shutdown:/sbin/swapoff -a
::shutdown:/bin/umount -a -r
```

```bash
# /etc/fstab
# <file system> <mount pt>      <type>  <options>       <dump>  <pass>
/dev/root       /               ext2    rw,noauto       0       1
proc            /proc           proc    defaults        0       0
devpts          /dev/pts        devpts  defaults,gid=5,mode=620,ptmxmode=0666   0       0
tmpfs           /dev/shm        tmpfs   mode=0777       0       0
tmpfs           /tmp            tmpfs   mode=1777       0       0
tmpfs           /run            tmpfs   mode=0755,nosuid,nodev  0       0
sysfs           /sys            sysfs   defaults        0       0
```