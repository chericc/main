# kernel_debug

## download kernel

```bash
# git clone 
git clone https://mirrors.ustc.edu.cn/linux.git/
# or download tgz
wget https://mirrors.tuna.tsinghua.edu.cn/kernel/v6.x/linux-6.12.tar.xz
```

## build and config

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

```bash
SYSTEM_TRUSTED_KEY=n
SYSTEM_REVOCATION_KEYS=n
```

## fs

```bash
qemu-system-x86_64 -kernel ../kernel/build/output/vmlinuz-6.12.0+ -m 512M -nographic -serial mon:stdio
```