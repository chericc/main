# ffmpeg

[TOC]

## 0. ffprobe

### 0.1 获取流列表

不用加任何参数即会打印流列表：

```bash
  Duration: 00:07:26.29, start: 0.000000, bitrate: 891 kb/s
  Stream #0:0: Video: h264 (High), yuv420p(tv, bt709/unknown/bt709, progressive), 852x480 [SAR 640:639 DAR 16:9], 24 fps, 24 tbr, 1k tbn
    Metadata:
      DURATION        : 00:07:26.249000000
  Stream #0:1: Audio: aac (LC), 48000 Hz, stereo, fltp
    Metadata:
      DURATION        : 00:07:26.293000000
```

其中，`Stream #0:1`表示这是第一个输入文件（序号为0）的第二个流（序号为1）；

### 0.2 获取流信息

```bash
-show_streams
```

### 0.3 所有编码分组

```bash
-show_packets
```

### 0.4 统计帧数

```bash
-count_frames
```

### 0.5 所有帧

```bash
-show_frames
```

### 0.6 像素格式

```bash
-show_pixel_formats
```

### 0.7 选择流

```bash
-select_streams v:0
```

### 0.8 打印格式

```bash
-pretty
```

## 1. ffmpeg命令基本形式

```bash
ffmpeg [global-options] {[input_file_options] -i input_url} ... {[output_file_options] output_url}
```

## 2. 流选择

### 2.1 从多个文件中选择特定的流

先用"ffprobe"获取流列表（参考ffprobe功能 ）；

比如现有两个源媒体文件"1.mkv"和"2.mkv"，要选取第一个文件的视频和第二个文件的音频，且两个文件的视频和音频通道的索引均为0,1，输出到目标文件"output.mkv"中，则命令为

```bash
ffmpeg -i 1.mkv -i 2.mkv -map 0:0 -map 1:1 output.mkv
```

### 2.2 屏蔽所有视频流

```bash
-vn
```

### 2.3 屏蔽所有音频流

```bash
-an
```

## 3. 时长

### 3.1 从某刻开始一定时间

```bash
-ss time_point -t duration
```

### 3.2 从某刻到某刻

```bash
-ss time_point1 -to time_point2
```

## 4. 视频编码

### 4.1 指定视频帧数量

```bash
-frames:v 2400
```

### 4.2 指定视频编码格式

```bash
-c:v hevc
```

### 4.3 指定帧率

```bash
-r 24
```

### 4.4 指定编码码率

```bash
-b:v 256k
```

### 4.5 缩放

```bash
-vf scale=640:-1
```

## 5. 音频编码

### 5.1 音频通道数

```bash
-ac 2
```

### 5.2 音频编码格式

```bash
-c:a libopus
```

### 5.3 码率

```bash
-b:a 96k
```

### 5.4 采样率

```bash
-ar 48000
```

## 6. 编码器特有选项

如：

libx265 AVOptions:
  -crf               <float>      E..V....... set the x265 crf (from -1 to FLT_MAX) (default -1)
  -qp                <int>        E..V....... set the x265 qp (from -1 to INT_MAX) (default -1)
  -forced-idr        <boolean>    E..V....... if forcing keyframes, force them as IDR frames (default false)
  -preset            <string>     E..V....... set the x265 preset
  -tune              <string>     E..V....... set the x265 tune parameter
  -profile           <string>     E..V....... set the x265 profile
  -udu_sei           <boolean>    E..V....... Use user data unregistered SEI if available (default false)
  -x265-params       <dictionary> E..V....... set the x265 configuration using a :-separated list of key=value parameters

## 7. yuv

### 7.1 编码YUV

```bash
ffmpeg -pix_fmt nv21 -s 640*360 -i output.yuv -r 20 output.mp4
```

### 7.2 解码为YUV

指定后缀名为YUV即可；

```bash
ffmpeg -i input.file output.yuv
```

## 8. 性能

### 8.1 线程

```bash
-threads
```

## 9. 其它

### 9.1 提取关键帧

```bash
ffmpeg -i test.mp4 -vf select='eq(pict_type\,I)' -vsync 2 -f image2 kf-%02d.bmp
```

## 10. 组播

```
ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i 8K-HEVC125M.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.3:5140
ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i 8K1_10s.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.3:5140

ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop 0 -i 8K1_10s.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.3:5140

input text udp://239.239.3.3:5140

input text rtsp://192.168.1.222/8K-HEVC125M.ts

```