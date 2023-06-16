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