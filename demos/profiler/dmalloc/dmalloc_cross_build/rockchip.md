# arm-rockchip-linux- 环境使用 dmalloc 调试

## 环境

### 编译

../dmalloc-5.6.5/configure \
    --prefix=$(pwd)/output \
    --host=arm-linux-gnueabihf \
    --target=arm-linux-gnueabihf \
    --enable-shlib \
    CFLAGS=-fPIC \
    CC=arm-rockchip-linux-gcc \
    CXX=arm-rockchip-linux-g++ \
    LD=arm-rockchip-linux-ld

make threadscxx
make installthcxx


### free 报错

export LD_PRELOAD=./libxxx.so

比如 

export LD_PRELOAD=./libPAR.so

看起来也没有效果