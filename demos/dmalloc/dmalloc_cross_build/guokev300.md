# arm-gcc6.3-linux-uclibceabi-g++ -v
Using built-in specs.
COLLECT_GCC=arm-gcc6.3-linux-uclibceabi-g++
COLLECT_LTO_WRAPPER=/opt/goke-linux/arm-gcc6.3-linux-uclibceabi/host_bin/../libexec/gcc/arm-linux-uclibceabi/6.3.0/lto-wrapper
Target: arm-linux-uclibceabi
Configured with: /home/durian/toolchains_ipc/source/build/script/arm-gcc6.3-linux-uclibceabi/arm-gcc6.3-linux-uclibceabi_build_dir/src/gcc-6.3.0/configure --host=x86_64-linux-gnu --build=x86_64-linux-gnu --target=arm-linux-uclibceabi --prefix=/home/durian/toolchains_ipc/source/build/script/arm-gcc6.3-linux-uclibceabi/arm-gcc6.3-linux-uclibceabi_build_dir/install --enable-threads --disable-libmudflap --enable-libssp --disable-libstdcxx-pch --with-gnu-as --with-gnu-ld --enable-languages=c,c++ --enable-shared --enable-lto --enable-symvers=gnu --enable-__cxa_atexit --disable-nls --enable-clocale=gnu --enable-extra-goke-multilibs --with-sysroot=/home/durian/toolchains_ipc/source/build/script/arm-gcc6.3-linux-uclibceabi/arm-gcc6.3-linux-uclibceabi_build_dir/install/target --with-build-sysroot=/home/durian/toolchains_ipc/source/build/script/arm-gcc6.3-linux-uclibceabi/arm-gcc6.3-linux-uclibceabi_build_dir/install/target --with-gmp=/home/durian/toolchains_ipc/source/build/script/arm-gcc6.3-linux-uclibceabi/arm-gcc6.3-linux-uclibceabi_build_dir/obj/host-libs/usr --with-mpfr=/home/durian/toolchains_ipc/source/build/script/arm-gcc6.3-linux-uclibceabi/arm-gcc6.3-linux-uclibceabi_build_dir/obj/host-libs/usr --with-mpc=/home/durian/toolchains_ipc/source/build/script/arm-gcc6.3-linux-uclibceabi/arm-gcc6.3-linux-uclibceabi_build_dir/obj/host-libs/usr --enable-libgomp --disable-libquadmath --disable-fixed-point --disable-libsanitizer --disable-libitm --enable-poison-system-directories --disable-bootstrap
Thread model: posix
gcc version 6.3.0 (GCC)

arm-gcc6.3-linux-uclibceabi-g++

../dmalloc-5.6.5/configure \
    --prefix=$(pwd)/output \
    --host=arm-linux-uclibceabi \
    --target=arm-linux-uclibceabi \
    CC=arm-gcc6.3-linux-uclibceabi-gcc \
    CXX=arm-gcc6.3-linux-uclibceabi-g++ \
    LD=arm-gcc6.3-linux-uclibceabi-ld \
    CFLAGS=-fPIC

make threadscxx

make installthcxx