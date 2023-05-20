# main

( 0_0 )

## How to build googletest?

```bash
cmake ../googletest-release-1.12.1/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug
```

## How to build ffmpeg?

```bash
# install pkg-config...

../ffmpeg/configure --prefix=$(pwd)/output --enable-shared --disable-static --disable-autodetect --disable-asm
```

## How to build SDL?

```bash
cmake ../SDL2-2.26.5/ -DCMAKE_INSTALL_PREFIX=$(pwd)/output -DSDL_STATIC=OFF -DCMAKE_BUILD_TYPE=Debug
```