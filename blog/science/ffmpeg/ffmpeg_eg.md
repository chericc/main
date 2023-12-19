# ffmpeg_eg

## 组播

```bash

./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i 8K-HEVC125M.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.3:5140


# loop
{
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i jlxiao/dump_20231030/8K/multicast/dump_igmp_ch25.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.3:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i jlxiao/dump_20231030/8K/multicast/dump_igmp_ch26.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.4:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i jlxiao/dump_20231030/8K/multicast/dump_4k_sanbeiji_50m50p8bit.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.5:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i jlxiao/dump_20231030/8K/multicast/dump_4k_yangjiang_5_fenzhong_50m50p8bit.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.6:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i jlxiao/dump_20231030/8K/multicast/dump_h265_4k_16m_60fps_3min_logo4.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.7:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i jlxiao/dump_20231030/8K/multicast/dump_h265_samsung_4k_20m_cbr_60fps.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.8:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i /opt/fonsview/data/media/movie/4K/HD.Club-4K-Chimei-inn-60mbps.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.9:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i jlxiao/problem/afamda_4K.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.10:5140&
  ./ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i demo2K.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.11:5140&
}

```

## 解决网卡丢失问题

```bash

网卡丢失后ffmpeg会报错退出，可以循环启动

while true
do 
ffmpeg -probesize 50M -analyzeduration 100M -re -stream_loop -1 -i 8K-HEVC125M.ts -c:a copy -c:v copy -f rtp_mpegts rtp://239.239.3.3:5140
sleep 5
done

```


## 转码

```bash


./ffmpeg -i 8K-World-\[H265-60p-CFR-150mbps\].mp4 -c:v libx265 -b:v 180M -x265-params pass=1:vbv-maxrate=180000:vbv-bufsize=24000 -c:a copy -f mpegts -y /dev/null
./ffmpeg -i 8K-World-\[H265-60p-CFR-150mbps\].mp4 -c:v libx265 -b:v 120M -x265-params pass=2:vbv-maxrate=120000:vbv-bufsize=24000 -c:a copy -f mpegts -y 8K-HEVC120M.ts

./ffmpeg -i 8K-World-\[H265-60p-CFR-150mbps\].mp4 -c:v libx265 -b:v 80M -x265-params pass=1:vbv-maxrate=80000:vbv-bufsize=18000 -c:a copy -f mpegts -y /dev/null
./ffmpeg -i 8K-World-\[H265-60p-CFR-150mbps\].mp4 -c:v libx265 -b:v 80M -x265-params pass=2:vbv-maxrate=80000:vbv-bufsize=18000 -c:a copy -f mpegts -y 8K-HEVC80M_custom.ts

./ffmpeg -i 8K-World-\[H265-60p-CFR-150mbps\].mp4 -c:v libx265 -b:v 80M -s 3840x2160 -c:a copy -f mpegts -y 8K-HEVC80M_custom_4K.ts

./ffmpeg -i 8KWorld120M.ts -c:v libx265 -b:v 80M -x265-params pass=1:vbv-maxrate=80000:vbv-bufsize=18000 -c:a copy -f mpegts -y /dev/null

```

## ffmpeg build

```bash

sudo apt-get install yasm libfdk-aac-dev libmp3lame-dev libopus-dev libsvtav1enc-dev libvorbis-dev libx264-dev libx265-dev libxavs2-dev -y 

## av1 vp9 uses HandBrake
../ffmpeg-6.1/configure --prefix=$(pwd)/output --disable-shared --enable-static --enable-gpl --enable-version3 --enable-nonfree --enable-libx264 --enable-libx265 --enable-libxavs2  --enable-libfdk-aac --enable-libopus --enable-libvorbis --enable-libmp3lame --disable-doc

VFORMAT_MPEG12, --> mpeg1video mpeg2video
VFORMAT_MPEG4, --> mpeg4
VFORMAT_H264, --> libx264
VFORMAT_MJPEG, --> mjpeg
VFORMAT_REAL, --> rv10 rv20
VFORMAT_JPEG, --> jpeg2000 ? jpegls ?
VFORMAT_VC1, --> vc2 ?
VFORMAT_AVS, --> libxavs(ubuntu源中没有，暂时不做)
VFORMAT_SW, --> ??
VFORMAT_H264MVC, --> ??
VFORMAT_H264_4K2K, --> ??
VFORMAT_HEVC, --> libx265
VFORMAT_H264_ENC, --> none
VFORMAT_JPEG_ENC, --> none
VFORMAT_VP9, --> libvpx_vp9
VFORMAT_AVS2, --> libxavs2
VFORMAT_AV1, --> 
VFORMAT_AVS3, --> Not support

AFORMAT_MPEG              = 0,    --> libmp3lame ?
AFORMAT_PCM_S16LE         = 1,    --> pcm_s16le
AFORMAT_AAC               = 2,    --> libfdk-aac
AFORMAT_AC3               = 3,    --> ac3
AFORMAT_ALAW              = 4,    --> pcm_alaw
AFORMAT_MULAW             = 5,    --> pcm_mulaw
AFORMAT_DTS               = 6,    --> dca ?
AFORMAT_PCM_S16BE         = 7,    --> pcm_s16be
AFORMAT_FLAC              = 8,    --> flac
AFORMAT_COOK              = 9,    --> ? (Real)
AFORMAT_PCM_U8            = 10,   --> pcm_u8
AFORMAT_ADPCM             = 11,   --> adpcm_g722 (多个)
AFORMAT_AMR               = 12,   --> not support
AFORMAT_RAAC              = 13,   --> ?
AFORMAT_WMA               = 14,   --> wmav1 wmav2
AFORMAT_WMAPRO            = 15,   --> ?
AFORMAT_PCM_BLURAY        = 16,   --> pcm_bluray
AFORMAT_ALAC              = 17,   --> alac
AFORMAT_VORBIS            = 18,   --> libvorbis
AFORMAT_AAC_LATM          = 19,   --> ?
AFORMAT_APE               = 20,   --> not support
AFORMAT_EAC3              = 21,   --> eac3
AFORMAT_PCM_WIFIDISPLAY   = 22,   --> none
AFORMAT_DRA               = 23,   --> ?
AFORMAT_SIPR              = 24,   --> not support
AFORMAT_TRUEHD            = 25,   --> truehd
AFORMAT_MPEG1             = 26,   --> not support
AFORMAT_MPEG2             = 27,   --> mp2 
AFORMAT_WMAVOI            = 28,   --> ?
AFORMAT_WMALOSSLESS       = 29,   --> ?
AFORMAT_PCM_S24LE         = 30,   --> pcm_s24le
AFORMAT_AV3A              = 31,   --> ?

```

```bash

# video encode 
ffmpeg -i 4K_Food.mkv -c:v mpeg1video -c:a libfdk_aac -y 4K_Food_MPEG1_AAC.mkv
ffmpeg -i 4K_Food.mkv -c:v mpeg2video -c:a libfdk_aac -y 4K_Food_MPEG2_AAC.mkv
ffmpeg -i 4K_Food.mkv -c:v mpeg4 -c:a libfdk_aac -y 4K_Food_MPEG4_AAC.mkv
ffmpeg -i 4K_Food.mkv -c:v libx264 -c:a libfdk_aac -y 4K_Food_H264_AAC.mkv
ffmpeg -i 4K_Food.mkv -c:v libx265 -c:a libfdk_aac -y 4K_Food_HEVC_AAC.mkv
ffmpeg -i 4K_Food.mkv -c:v mjpeg -c:a libfdk_aac -y 4K_Food_MJPEG_AAC.mkv
ffmpeg -i 4K_Food.mkv -vf scale=1280:-1 -c:v rv10 -c:a libfdk_aac -y 4K_Food_RV10_w1280_AAC.rm
ffmpeg -i 4K_Food.mkv -vf scale=1280:-1 -c:v rv20 -c:a libfdk_aac -y 4K_Food_RV20_w1280_AAC.rm
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v jpeg2000 -c:a libfdk_aac -y 4K_Food_JPEG2000_w640_AAC.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v jpegls -c:a libfdk_aac -y 4K_Food_JPEGLS_w640_AAC.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v vc2 -c:a libfdk_aac -y 4K_Food_VC2_w640_AAC.mkv
ffmpeg -i 4K_Food.mkv -c:v libxavs2 -c:a libfdk_aac -y 4K_Food_AVS2_AAC.mkv

# audio encode
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a libmp3lame -y 4K_Food_HEVC_w640_MP3.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a pcm_s16le -y 4K_Food_HEVC_w640_PCMS16LE.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a ac3 -y 4K_Food_HEVC_w640_AC3.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a pcm_alaw -y 4K_Food_HEVC_w640_PCMALAW.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a pcm_mulaw -y 4K_Food_HEVC_w640_PCMMULAW.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a pcm_s16be -y 4K_Food_HEVC_w640_PCMS16BE.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a flac -y 4K_Food_HEVC_w640_FLAC.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a adpcm_g722 -y 4K_Food_HEVC_w640_ADPCMG722.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a wmav1 -y 4K_Food_HEVC_w640_WMAV1.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a wmav2 -y 4K_Food_HEVC_w640_WMAV2.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a alac -y 4K_Food_HEVC_w640_ALAC.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a libvorbis -y 4K_Food_HEVC_w640_VORBIS.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a eac3 -y 4K_Food_HEVC_w640_EAC3.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a mp2 -y 4K_Food_HEVC_w640_MP2.mkv
ffmpeg -i 4K_Food.mkv -vf scale=640:-1 -c:v libx265 -c:a pcm_s24le -y 4K_Food_HEVC_w640_PCMS24LE.mkv

# wrapper
4K_Food_AV1_MP2.mkv              4K_Food_HEVC_w640_MP3.mkv       4K_Food_JPEGLS_w640_AAC.mkv
4K_Food_AVS2_AAC.mkv             4K_Food_HEVC_w640_PCMALAW.mkv   4K_Food_MJPEG_AAC.mkv
4K_Food_H264_AAC.mkv             4K_Food_HEVC_w640_PCMMULAW.mkv  4K_Food.mkv
4K_Food_H264_MP2.mkv             4K_Food_HEVC_w640_PCMS16BE.mkv  4K_Food_MPEG1_AAC.mkv
4K_Food_H265_MP2.mkv             4K_Food_HEVC_w640_PCMS16LE.mkv  4K_Food_MPEG2_AAC.mkv
4K_Food_HEVC_w640_AC3.mkv        4K_Food_HEVC_w640_PCMS24LE.mkv  4K_Food_MPEG4_AAC.mkv
4K_Food_HEVC_w640_ADPCMG722.mkv  4K_Food_HEVC_w640_PCMU8.mkv     4K_Food_VC2_w640_AAC.mkv
4K_Food_HEVC_w640_ALAC.mkv       4K_Food_HEVC_w640_VORBIS.mkv    4K_Food_VP8_MP2.mkv
4K_Food_HEVC_w640_EAC3.mkv       4K_Food_HEVC_w640_WMAV1.mkv     4K_Food_VP9_MP2.mkv
4K_Food_HEVC_w640_FLAC.mkv       4K_Food_HEVC_w640_WMAV2.mkv
4K_Food_HEVC_w640_MP2.mkv        4K_Food_JPEG2000_w640_AAC.mkv

for i in `ls ./`
do
echo "processing $i"
ffmpeg -i $1 -codec copy $
done

ffmpeg -i 4K_Food_AV1_MP2.mkv

```