# xmedia
some media codes


## build ffmpeg

```bash
../ffmpeg/configure --prefix=$(pwd)/output  --disable-x86asm --disable-stripping --disable-static --enable-shared --disable-autodetect
```

## generate video samples

```bash
ffmpeg -i qianxun.mkv -to 06:30 -c:v libx264 -c:a copy -map 0:0 -map 0:1 -vf scale=640:320 -ss 06:00 output_qianxun_640_360.mkv
```

## easy run

```bash
./xplay C:\Users\test\Videos\demo.mkv
```