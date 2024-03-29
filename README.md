# main

( 0_0 )

## Ubuntu setup

```bash

sudo apt-get install bzip2 -y
sudo apt-get install build-essential -y
sudo apt-get install pkg-config  -y

```

## Some 3rd libraries

```bash
# GoogleTest
## googletest 1.12.x is the last version support c++11
cmake ../googletest-release-1.12.1/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14

# FFMpeg
## install pkg-config...
../ffmpeg/configure --prefix=$(pwd)/output --enable-shared --disable-static --disable-autodetect --disable-asm
../ffmpeg-4.4.4/configure --prefix=$(pwd)/output --enable-shared --disable-static --disable-autodetect --disable-asm

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
../valgrind-3.20.0/configure --prefix=$(pwd)/output

# libunwind
## http://download.savannah.gnu.org/releases/libunwind/
../libunwind-1.6.2/configure --prefix=$(pwd)/output

# gperf-tools
## without libunwind
cmake ../gperftools-2.10 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGPERFTOOLS_BUILD_STATIC=OFF -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=OFF
## with libunwind
## note: libunwind version is restricted
CMAKE_LIBRARY_PATH=/home/test/opensrc/libunwind/build/output/lib CMAKE_INCLUDE_PATH=/home/test/opensrc/libunwind/build/output/include cmake ../gperftools-2.10 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGPERFTOOLS_BUILD_STATIC=OFF -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=ON
## run uni-test
make test
## note: uni-test failed, not knowing why.
## libunwind is recommended

# benchmark
cmake ../benchmark-1.8.0/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGOOGLETEST_PATH=/home/test/opensrc/googletest/googletest-release-1.12.1

# Abseil
## Google's common libraries(C++)
cmake ../abseil-cpp/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DABSL_PROPAGATE_CXX_STD=ON

# plantuml
sudo sh -c 'echo "java -Djava.awt.headless=true -jar /home/test/opensrc/plantuml/plantuml.1.2023.7.jar \$*" > /usr/bin/plantuml'
sudo chmod 775 /usr/bin/plantuml

# iperf3
../iperf-3.1.3/configure --prefix=$(pwd)/output

# irqbalance
# irqbalance-1.9.2 >
./autogen.sh
# build > 
../irqbalance-1.9.2/configure --prefix=$(pwd)/output --without-irqbalance-ui
```
