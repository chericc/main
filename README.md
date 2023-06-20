# main

( 0_0 )

## How to build googletest?

```bash
# googletest 1.12.x is the last version support c++11
cmake ../googletest-release-1.12.1/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14
make
make install
```

## How to build ffmpeg?

```bash
# install pkg-config...
../ffmpeg/configure --prefix=$(pwd)/output --enable-shared --disable-static --disable-autodetect --disable-asm
make
make install
```

## How to build SDL?

windows or macos

```bash
cmake ../SDL2-2.26.5/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DSDL_STATIC=OFF -DCMAKE_BUILD_TYPE=Release
make
make install
```

ubuntu

```bash
sudo apt-get install libsdl2...(dev / 2.x)
```

## How to build freetype?

```bash
cmake ../freetype-2.10.2/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=true -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_ZLIB=ON -DFT_REQUIRE_BROTLI=ON -DFT_REQUIRE_BZIP2=ON -DFT_REQUIRE_HARFBUZZ=ON
make
make install
```

## How to build cmake?

```bash
# downlaod cmake
wget https://cmake.org/files/v3.26/cmake-3.26.4.tar.gz
../cmake-3.26.4/configure
make
make install
```

## How to build openssl?

```bash
../openssl-3.1.0/Configure
make
make install
```

## How to install plantuml?

```bash
sudo apt-get install openjdk-19-jdk
sudo apt-get install graphviz
sudo apt-get install plantuml
```

## How to build valgrind

```bash
../valgrind-3.20.0/configure --prefix=$(pwd)/output
```

## How to build libunwind

```bash
# http://download.savannah.gnu.org/releases/libunwind/

../libunwind-1.3.2/configure --prefix=$(pwd)/output CFLAGS=-fcommon

../libunwind-1.6.2/configure --prefix=$(pwd)/output
```

## How to build gperftools

```bash
# without libunwind
cmake ../gperftools-2.10 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGPERFTOOLS_BUILD_STATIC=OFF -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=OFF

# with libunwind
# note: libunwind version is restricted
CMAKE_LIBRARY_PATH=/home/test/opensrc/libunwind/build/output/lib CMAKE_INCLUDE_PATH=/home/test/opensrc/libunwind/build/output/include cmake ../gperftools-2.10 -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DGPERFTOOLS_BUILD_STATIC=OFF -Dgperftools_enable_frame_pointers=ON -Dgperftools_enable_libunwind=OFF

# run uni-test
make test

# note: uni-test failed, not knowing why.
```