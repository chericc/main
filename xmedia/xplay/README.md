# xmedia
some media codes


## build ffmpeg

```bash
../ffmpeg/configure --prefix=$(pwd)/output  --disable-x86asm --disable-stripping --disable-static --enable-shared --disable-autodetect
```

## generate video samples

```bash
ffmpeg -i waizhuan.mkv -strict -2 -c:v mpeg4 -c:a opus -ab 32k -map 0:0 -map 0:1 -vf scale=640:360 -ss 00:08 -to 00:28 demo.mp4
```

## easy run

```bash
./xplay C:\Users\test\Videos\demo.mkv
```