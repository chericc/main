# main

( 0_0 )

## Compile cmds

```bash
export OPENSRC_LIB_PATH=~/opensrc
export CMAKE_INCLUDE_PATH=$OPENSRC_LIB_PATH/ffmpeg/build/output/include:$OPENSRC_LIB_PATH/libwebsockets/build/output/include:$OPENSRC_LIB_PATH/live555/live/output/include
export CMAKE_LIBRARY_PATH=$OPENSRC_LIB_PATH/ffmpeg/build/output/lib:$OPENSRC_LIB_PATH/libwebsockets/build/output/lib:$OPENSRC_LIB_PATH/live555/live/output/lib
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## or

```bash
# may have version compatibility problem
sudo apt install \
libsdl2-dev \
libavformat-dev \
libavcodec-dev \
libavutil-dev \
libswresample-dev \
libswscale-dev \
libfreetype-dev \
libgtest-dev \
libwebsockets-dev \
libfdk-aac-dev \
libevent-dev \
libcurl4-openssl-dev \
libcjson-dev -y
```

## Ubuntu setup

```bash

sudo apt-get install bzip2 -y
sudo apt-get install build-essential -y
sudo apt-get install pkg-config  -y

```

## cross build for configure

```bash

CC="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" CXX="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++"
CFLAGS="-mcpu=cortex-a7 -mfpu=neon-vfpv4" ../dir/configuire --prefix=$(pwd)/output

```

## Some 3rd libraries

```bash
# GoogleTest
## googletest 1.12.x is the last version support c++11
cmake ../googletest/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14

# FFMpeg
## install pkg-config...
../ffmpeg/configure --prefix=$(pwd)/output --enable-shared --disable-static --disable-autodetect --disable-asm --disable-optimizations --disable-stripping --enable-debug=3

# SDL2
cmake ../SDL2-2.26.5/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DSDL_STATIC=OFF -DCMAKE_BUILD_TYPE=Release

# FreeType
cmake ../freetype-2.10.2/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=true -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_ZLIB=ON -DFT_REQUIRE_BROTLI=ON -DFT_REQUIRE_BZIP2=ON -DFT_REQUIRE_HARFBUZZ=ON

# CMake
wget https://cmake.org/files/v3.26/cmake-3.26.4.tar.gz
../cmake-3.26.4/configure

# OpenSSL
../openssl-3.1.0/Configure

# PlantUML
sudo apt-get install openjdk-19-jdk
sudo apt-get install graphviz
`sudo apt-get install plantuml`

# valgrind
../valgrind-3.23.0/configure --prefix=$(pwd)/output
export CC="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" 
export CXX="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++" 
export CFLAGS="-rdynamic -mcpu=cortex-a7 -mfpu=neon-vfpv4" 
../valgrind-3.23.0/configure --prefix=$(pwd)/output --enable-only32bit --host=arm-linux-gnueabihf
../valgrind-3.23.0/configure --prefix=$(pwd)/output --host=arm-linux-gnueabihf

# libunwind
## http://download.savannah.gnu.org/releases/libunwind/
../libunwind-1.6.2/configure --prefix=$(pwd)/output
## cross build
export CC="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" 
export CXX="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++" 
export CFLAGS="-rdynamic -mcpu=cortex-a7 -mfpu=neon-vfpv4 -fPIC"
export LIBS="-pthread"
../libunwind-1.8.1/configure --prefix=$(pwd)/output --host=arm-linux-gnueabihf --disable-tests --enable-shared=no --enable-cxx-exceptions
## cross-mips
export CC="/opt/mips-gcc720-glibc229/bin/mips-linux-gnu-gcc" 
export CXX="/opt/mips-gcc720-glibc229/bin/mips-linux-gnu-g++" 
../libunwind-1.8.1/configure --prefix=$(pwd)/output --host=mips --disable-tests --enable-shared=no --enable-cxx-exceptions

# gperf-tools
## without libunwind(works well)
cmake ../gperftools -DCMAKE_INSTALL_PREFIX=$(pwd)/output -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=OFF -DCMAKE_TOOLCHAIN_FILE=~/cross_823c.cmake
## with libunwind
## note: libunwind version is restricted
CMAKE_LIBRARY_PATH=/home/test/opensrc/libunwind/build/output/lib CMAKE_INCLUDE_PATH=/home/test/opensrc/libunwind/build/output/include cmake ../gperftools-gperftools-2.15 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGPERFTOOLS_BUILD_STATIC=OFF -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=ON -DCMAKE_TOOLCHAIN_FILE=~/cross_823c.cmake
## cross
CMAKE_LIBRARY_PATH=/home/test/opensrc/libunwind/build/output/lib CMAKE_INCLUDE_PATH=/home/test/opensrc/libunwind/build/output/include cmake ../gperftools -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGPERFTOOLS_BUILD_STATIC=OFF -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=ON -DCMAKE_TOOLCHAIN_FILE=~/cross_823c.cmake
## cross-ad100
CMAKE_LIBRARY_PATH=/home/test/opensrc/libunwind/build-ad100/output/lib CMAKE_INCLUDE_PATH=/home/test/opensrc/libunwind/build-ad100/output/include cmake ../gperftools -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGPERFTOOLS_BUILD_STATIC=OFF -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=ON -DCMAKE_TOOLCHAIN_FILE=~/ad100.cmake -DBUILD_TESTING=OFF

## run uni-test
make test
## note: uni-test failed, not knowing why.
## libunwind is recommended

# dmalloc
export CC="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" 
export CXX="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++" 
export CFLAGS="-rdynamic -mcpu=cortex-a7 -mfpu=neon-vfpv4 -fPIC"
export CXXFLAGS="-rdynamic -mcpu=cortex-a7 -mfpu=neon-vfpv4 -fPIC" 
../dmalloc-5.6.5/configure --prefix=$(pwd)/output --host=arm-linux-gnueabihf --enable-shlib
## static into shared
/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc  -L. -Wl,--whole-archive -ldmallocthcxx -Wl,--no-whole-archive -fPIC -shared -o tmp/libdmallocthcxx.so

# benchmark
cmake ../benchmark-1.8.0/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGOOGLETEST_PATH=/home/test/opensrc/googletest/googletest-release-1.12.1

# Abseil
## Google's common libraries(C++)
cmake ../abseil-cpp/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DABSL_PROPAGATE_CXX_STD=ON

# plantuml
sudo sh -c 'echo "java -Djava.awt.headless=true -jar /home/test/opensrc/plantuml/plantuml.1.2023.7.jar \$*" > /usr/bin/plantuml'
sudo chmod 775 /usr/bin/plantuml

# iperf3
env \
CC=/opt/mips-gcc720-glibc229/bin/mips-linux-gnu-gcc \
../iperf-3.17.1/configure --prefix=$(pwd)/output --host=mips --prefix=$(pwd)/output

# irqbalance
# irqbalance-1.9.2 >
./autogen.sh
# build > 
../irqbalance-1.9.2/configure --prefix=$(pwd)/output --without-irqbalance-ui

# libevent
sudo apt-get install libssl-dev doxygen
cmake ../libevent-2.1.12-stable -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DEVENT__LIBRARY_TYPE=SHARED -DCMAKE_BUILD_TYPE=Release -DEVENT__DOXYGEN=ON

# cjson
cmake ../cJSON-1.7.15 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DENABLE_CJSON_UTILS=ON

# iperf
## cross
export CC="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" 
export CFLAGS="-rdynamic -mcpu=cortex-a7 -mfpu=neon-vfpv4 -fPIC"
export LIBS="-pthread"
../iperf-3.17.1/configure --prefix=$(pwd)/output --host=arm-linux-gnueabihf --enable-static-bin
## x86
../iperf-3.17.1/configure --prefix=$(pwd)/output --enable-static-bin

# libcurl
../curl-7.55.0/configure --prefix=$(pwd)/output 

# media-server
copy from https://github.com/ireader/media-server

# libwebsocket
sudo apt install zlib1g-dev
cmake ../libwebsockets-4.3.3 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DCMAKE_BUILD_TYPE=Release -DLWS_WITH_SSL=OFF -DLWS_WITH_MINIMAL_EXAMPLES=OFF -DLWS_WITHOUT_EXTENSIONS=OFF -DLWS_ROLE_MQTT=ON

# live555
# cp config.linux config.myconfig
# chmod 664 ./config.myconfig
COMPILE_OPTS = -DNO_OPENSSL=1 -DNO_STD_LIB=1
LIBS_FOR_CONSOLE_APPLICATION = 
PREFIX = /home/test/opensrc/live555/live/output

./genMakefiles myconfig
mkdir output
make && make install

# mosquitto
cmake ../mosquitto-2.0.20 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DDOCUMENTATION=OFF

# nanomsg
cmake ../nanomsg-1.2.1/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DNN_ENABLE_DOC=OFF
## demo
export CMAKE_PREFIX_PATH=/home/test/opensrc/nanomsg/build/output/

# zeromq
cmake . -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DBUILD_SHARED=OFF -DZMQ_BUILD_TESTS=OFF -DWITH_PERF_TOOL=OFF -DWITH_LIBSODIUM=OFF -DWITH_LIBBSD=OFF -DBUILD_SHARED=OFF -DENABLE_CLANG=OFF -DENABLE_CPACK=OFF -DBUILD_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=~/cross_823c.cmake

# haveged
## generate random infomation for linux.
export CC="/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" 
export CFLAGS="-rdynamic -mcpu=cortex-a7 -mfpu=neon-vfpv4 -fPIC"
export LIBS="-pthread"
../haveged-1.9.19/configure --prefix=$(pwd)/output --disable-shared --host=arm-linux-gnueabihf


# dbus-1
sudo apt install libexpat1-dev -y
cmake ../dbus-dbus-1.14/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(pwd)/output

# cross
cmake ../dbus-dbus-1.14/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DCMAKE_TOOLCHAIN_FILE=~/cross_823c.cmake

# libnl-3.0
wget https://www.infradead.org/~tgr/libnl/files/libnl-3.2.25.tar.gz
env CC=/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc ./configure --prefix=$(pwd)/output --host=arm-linux-gnueabihf --enable-shared=no

# wpa_supplicant
wget https://w1.fi/releases/wpa_supplicant-2.9.tar.gz
## comment out some unnecessary options
## 
## src/drivers/drivers.mak
## before 'ifdef CONFIG_LIBNL32', add
## DRV_LIBS += $(shell $(PKG_CONFIG) --libs libnl-3.0)
## DRV_LIBS += $(shell $(PKG_CONFIG) --libs libnl-genl-3.0)
env PKG_CONFIG_PATH=/home/test/opensrc/libnl/libnl-3.2.25/output/lib/pkgconfig:/home/test/opensrc/dbus/build/output/lib/pkgconfig:$PKG_CONFIG_PATH pkg-config --modversion libnl-3.0 
env PKG_CONFIG_PATH=/home/test/opensrc/libnl/libnl-3.2.25/output/lib/pkgconfig:/home/test/opensrc/dbus/build/output/lib/pkgconfig:$PKG_CONFIG_PATH pkg-config --libs libnl-3.0 
env PKG_CONFIG_PATH=/home/test/opensrc/libnl/libnl-3.2.25/output/lib/pkgconfig:/home/test/opensrc/dbus/build/output/lib/pkgconfig:$PKG_CONFIG_PATH pkg-config --libs libnl-genl-3.0
env CC=/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc LD=/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ld LIBS="-lpthread -lm $LIBS" PKG_CONFIG_PATH=/home/test/opensrc/libnl/libnl-3.2.25/output/lib/pkgconfig:/home/test/opensrc/dbus/build/output/lib/pkgconfig:$PKG_CONFIG_PATH make 

# dosfstools
../dosfstools-4.1/configure --prefix=$(pwd)/output

# speexdsp
../speexdsp-1.2.1/configure --prefix=$(pwd)/output --enable-examples
--disable-examples

# nanomsg
cmake ../nanomsg/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DNN_ENABLE_DOC=OFF -DNN_STATIC_LIB=ON -DNN_TESTS=OFF

# c-ares
cmake ../c-ares-1.34.5 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DCMAKE_BUILD_TYPE=Release -DCARES_STATIC=ON -DCARES_SHARED=OFF

# boost
cmake ../boost-1.89.0 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DCMAKE_BUILD_TYPE=Release

# nginx
sudo apt install libssl-dev libpcre2-dev libxml2-dev libxslt-dev -y
./configure --prefix=$(pwd)/output --with-http_ssl_module --with-http_slice_module --with-http_dav_module --add-module=../ext_module/nginx-dav-ext-module/
# nginx cross build notes:
# auto/cc/name: ngx_feature_run=no
# auto/types/sizeof: ngx_size=`$NGX_AUTOTEST` --> ngx_size=4
# 
# echo "#ifndef NGX_HAVE_SYSVSHM
# define NGX_HAVE_SYSVSHM 1
# endif" >> objs/ngx_auto_config.h
#
export MY_PREBUILT_DIR=../../../prebuilt
export MY_CC=aarch64-none-linux-gnu-gcc
export MY_CPP=aarch64-none-linux-gnu-g++
export MY_SSL_DIR=/home/test/opensrc/openssl/output
export MY_SSL_INC_DIR=$MY_SSL_DIR/include
export MY_SSL_LIB_DIR=$MY_SSL_DIR/lib
export MY_PCRE_DIR=$MY_PREBUILT_DIR/pcre2/output
export MY_PCRE_INC_DIR=$MY_PCRE_DIR/include
export MY_PCRE_LIB_DIR=$MY_PCRE_DIR/lib
export MY_ZLIB_DIR=$MY_PREBUILT_DIR/zlib/output
export MY_ZLIB_INC_DIR=$MY_ZLIB_DIR/include
export MY_ZLIB_LIB_DIR=$MY_ZLIB_DIR/lib
PATH=$PATH:/opt/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin ./configure \
--prefix=$(pwd)/output --with-http_ssl_module --with-http_slice_module --with-http_dav_module \
--without-http_upstream_zone_module --without-stream_upstream_zone_module \
--with-cc=$MY_CC --with-cpp=$MY_CPP \
--with-cc-opt="-I$MY_SSL_INC_DIR -I$MY_PCRE_INC_DIR -I$MY_ZLIB_INC_DIR" \
--with-ld-opt="-L$MY_SSL_LIB_DIR -L$MY_PCRE_LIB_DIR -L$MY_ZLIB_LIB_DIR"
```

