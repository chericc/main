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

make KBUILD_SRC=../busybox-1.36.1 -f ../busybox-1.36.1/Makefile defconfig
make CONFIG_PREFIX=./output install 

```


## rootfs

### build rootfs

```bash
# create disk
qemu-img create -f raw disk.img 1G
# create partition
fdisk ./disk.img
# after partition create
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
# copy fs to disk's root partition(default sda)
dd if=rootfs.squashfs of=disk.img bs=1 count=4096 seek=2048
# 
qemu-system-x86_64 -kernel ../kernel/build/output/vmlinuz-6.12.0-10553-gb86545e02e8c -m 512M -nographic -serial mon:stdio -append "console=ttyS0,root=/dev/sda1" -drive file=./disk.img,format=raw
```

### fs

```bash
mkdir  dev  etc  home  lib   mnt  proc  root   sys  tmp   var -p

qemu-system-x86_64 -kernel ../kernel/build/output/vmlinuz-6.12.0-10553-gb86545e02e8c -m 512M -nographic -serial mon:stdio -append "console=ttyS0"
```