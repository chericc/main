# ffmpeg

## common

### 不打印配置信息

```bash

ffmpeg -loglevel quiet

```

## ffprobe

### 获取流列表

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

### 获取流信息

```bash
-show_streams
```

### 所有编码分组

```bash
-show_packets
```

### 统计帧数

```bash
-count_frames
```

### 所有帧

```bash
-show_frames
```

### 像素格式

```bash
-show_pixel_formats
```

### 选择流

```bash
-select_streams v:0
```

### 打印格式

```bash
-pretty
```

## ffplay

### url

```bash
ffplay [url] -x 640 -y 320
```

## ffmpeg

### ffmpeg命令基本形式

```bash
ffmpeg [global-options] {[input_file_options] -i input_url} ... {[output_file_options] output_url}
```

### 封装形式

```bash
# mp4 --> mkv
ffmpeg -i video.mp4 video.mkv
```

### 流选择

#### 从多个文件中选择特定的流

先用"ffprobe"获取流列表（参考ffprobe功能 ）；

比如现有两个源媒体文件"1.mkv"和"2.mkv"，要选取第一个文件的视频和第二个文件的音频，且两个文件的视频和音频通道的索引均为0,1，输出到目标文件"output.mkv"中，则命令为

```bash
ffmpeg -i 1.mkv -i 2.mkv -map 0:0 -map 1:1 output.mkv
```

#### 屏蔽所有视频流

```bash
-vn
```

#### 屏蔽所有音频流

```bash
-an
```

### 时长

#### 从某刻开始一定时间

```bash
-ss time_point -t duration
```

#### 从某刻到某刻

```bash
-ss time_point1 -to time_point2
```

### 视频编码

#### 指定视频帧数量

```bash
-frames:v 2400
```

#### 指定视频编码格式

```bash
-c:v hevc
-c:v copy

-codec copy
```

#### 指定帧率

```bash
-r 24
```

#### 指定编码码率

```bash
-b:v 256k
```

### 音频编码

#### 音频通道数

```bash
-ac 2
```

#### 音频编码格式

```bash
-c:a libopus
```

#### 码率

```bash
-b:a 96k
```

#### 采样率

```bash
-ar 48000
```

#### 声道

```bash
-ac 1
```

### 编码器特有选项

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

### yuv

#### 编码YUV

```bash
ffmpeg -pix_fmt nv21 -s 640*360 -i output.yuv -r 20 output.mp4
```

#### 解码为YUV

指定后缀名为YUV即可；

```bash
ffmpeg -i input.file output.yuv
```

### 性能

#### 线程

```bash
-threads
```

### 其它

#### 提取关键帧

```bash
ffmpeg -i test.mp4 -vf select='eq(pict_type\,I)' -vsync 2 -f image2 kf-%02d.bmp
```

#### 检查关键帧信息

```bash
ffmpeg -i input.mp4 -vf showinfo -f null -
```

#### m3u8

```bash
ffmpeg -i ..\Left_Right_MIX.mp4 -c copy -f hls 111.m3u8
```

### 组播

```bash
./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i 8K-HEVC41M.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.3:5140
```

### 滤镜



#### 缩放

```bash
-vf scale=640:-1
-filter:v scale=640:-1
```

#### 制作黑边

```bash

-vf pad=1920:1080:0:100:blue

其中0，100指视频放置的位置，1920，1080指最终视频大小

```

### x264编码参数


```bash
# simulate IPC encoders
ffmpeg -i h264_1080p.h264 -c:v libx264 \
-profile:v high -level 5.1 -pix_fmt yuv420p \
-r 20 -s 1280x720 \
-x264-params "ref=1:keyint=40:min-keyint=40:bframes=0" \
output.h264
```