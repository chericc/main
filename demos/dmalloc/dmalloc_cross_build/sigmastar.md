../dmalloc-5.6.5/configure \
    --prefix=$(pwd)/output \
    --host=arm-buildroot-linux-uclibcgnueabihf \
    --target=arm-buildroot-linux-uclibcgnueabihf \
    CC=arm-buildroot-linux-uclibcgnueabihf-gcc \
    CXX=arm-buildroot-linux-uclibcgnueabihf-g++ \
    LD=arm-buildroot-linux-uclibcgnueabihf-ld \
    CFLAGS=-fPIC

make threadscxx

make installthcxx