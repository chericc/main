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


## special encodes

```bash
ffmpeg -help encoder=libsvtav1
ffmpeg -help encoder=libaom-av1 
ffmpeg -help encoder=libx265
ffmpeg -help encoder=libxavs2
ffmpeg -help encoder=libx264

```


```bash
## 

ffmpeg -i 4K_food.ts -codec copy -t 30 Food.mkv

## video
ffmpeg -i Food.mkv -an -vcodec libx264 -vf scale=640:360 -r 60 -b:v 50M -y Food_videosrc.mkv
ffmpeg -i Food.mkv -vn -c:a copy Food_audiosrc.mkv

ffmpeg -i Food_videosrc.mkv -an -vcodec mpeg1video -b:v 2M -y Food_MPEG1.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec mpeg2video -b:v 2M -y Food_MPEG2.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec libx264 -profile:v high -level 5.1 -b:v 2M -y Food_H264_HIGH_51.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec mjpeg -b:v 2M -y Food_MJPEG.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec rv10 -vf scale=640:320 -b:v 2M -y Food_RV10.rm
ffmpeg -i Food_videosrc.mkv -an -vcodec rv20 -vf scale=640:320 -b:v 2M -y Food_RV20.rm
ffmpeg -i Food_videosrc.mkv -an -vcodec libx265 -profile:v main10 -x265-params level-idc=6.1 -b:v 2M -y Food_HEVC_MAIN10_61.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec libvpx-vp9 -b:v 2M Food_VP9.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec libxavs2 -b:v 2M Food_AVS2.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec libsvtav1 -b:v 2M Food_AV1.mkv
ffmpeg -i Food_videosrc.mkv -an -vcodec mpeg4 -b:v 2M Food_MPEG4.mkv

ffmpeg -i Food_audiosrc.mkv -i Food_MPEG1.mkv -codec copy -y Food_MPEG1_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_MPEG2.mkv -codec copy -y Food_MPEG2_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_H264_HIGH_51.mkv -codec copy -y Food_H264_HIGH_51_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_MJPEG.mkv -codec copy -y Food_MJPEG_.mkv
# ffmpeg -i Food_audiosrc.mkv -i Food_RV10.rm -codec copy -y Food_RV10_.rm
# ffmpeg -i Food_audiosrc.mkv -i Food_RV20.rm -codec copy -y Food_RV20_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_HEVC_MAIN10_61.mkv -codec copy -y Food_HEVC_MAIN10_61_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_VP9.mkv -codec copy -y Food_VP9_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_AVS2.mkv -codec copy -y Food_AVS2_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_AV1.mkv -codec copy -y Food_AV1_.mkv
ffmpeg -i Food_audiosrc.mkv -i Food_MPEG4.mkv -codec copy -y Food_MPEG4_.mkv

## pure audio
ffmpeg -i Food.mkv -vn -c:a copy Food_audiosrc.mkv
ffmpeg -i Food.mkv -an -vcodec libx264 -vf scale=640:360 -r 60 -b:v 2M -y Food_videosrc.mkv

ffmpeg -i Food_audiosrc.mkv -c:a libmp3lame -y Food_MP3.mkv
ffmpeg -i Food_audiosrc.mkv -c:a pcm_s16le -y Food_PCMS16LE.mkv
ffmpeg -i Food_audiosrc.mkv -c:a aac -y Food_AAC.mkv
ffmpeg -i Food_audiosrc.mkv -c:a ac3 -y Food_AC3.mkv
ffmpeg -i Food_audiosrc.mkv -c:a pcm_alaw -y Food_PCMALAW.mkv
ffmpeg -i Food_audiosrc.mkv -c:a pcm_mulaw -y Food_PCMMULAW.mkv
ffmpeg -i Food_audiosrc.mkv -c:a dca -strict -2 -y Food_DTS.mkv
ffmpeg -i Food_audiosrc.mkv -c:a pcm_s16be -y Food_PCMS16BE.mkv
ffmpeg -i Food_audiosrc.mkv -c:a flac -y Food_FLAC.mkv
ffmpeg -i Food_audiosrc.mkv -c:a pcm_u8 -y Food_PCM_U8.mkv
ffmpeg -i Food_audiosrc.mkv -c:a adpcm_ms -y Food_ADPCM_MS.mkv
ffmpeg -i Food_audiosrc.mkv -c:a libopencore_amrnb -ar 8k -b:a 12.2k -ac 1 -y Food_AMR_NB.mkv
ffmpeg -i Food_audiosrc.mkv -c:a libvo_amrwbenc -ar 16k -b:a 23.85k -ac 1 -y Food_AMR_WB.mkv
ffmpeg -i Food_audiosrc.mkv -c:a wmav1 -y Food_WMAV1.mkv
ffmpeg -i Food_audiosrc.mkv -c:a wmav2 -y Food_WMAV2.mkv
ffmpeg -i Food_audiosrc.mkv -c:a alac -y Food_ALAC.mkv
ffmpeg -i Food_audiosrc.mkv -c:a libvorbis -y Food_VORBIS.mkv
ffmpeg -i Food_audiosrc.mkv -c:a eac3 -y Food_EAC3.mkv
ffmpeg -i Food_audiosrc.mkv -c:a truehd -strict -2 -y Food_TRUEHD.mkv
ffmpeg -i Food_audiosrc.mkv -c:a mp2 -y Food_MP2.mkv
ffmpeg -i Food_audiosrc.mkv -c:a pcm_s24le -y Food_PCMS24LE.mkv

ffmpeg -i Food_videosrc.mkv -i Food_MP3.mkv -codec copy -y Food_MP3_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_PCMS16LE.mkv -codec copy -y Food_PCMS16LE_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_AAC.mkv -codec copy -y Food_AAC_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_AC3.mkv -codec copy -y Food_AC3_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_PCMALAW.mkv -codec copy -y Food_PCMALAW_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_PCMMULAW.mkv -codec copy -y Food_PCMMULAW_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_DTS.mkv -codec copy -y Food_DTS_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_PCMS16BE.mkv -codec copy -y Food_PCMS16BE_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_FLAC.mkv -codec copy -y Food_FLAC_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_PCM_U8.mkv -codec copy -y Food_PCM_U8_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_ADPCM_MS.mkv -codec copy -y Food_ADPCM_MS_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_AMR_NB.mkv -codec copy -y Food_AMR_NB_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_AMR_WB.mkv -codec copy -y Food_AMR_WB_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_WMAV1.mkv -codec copy -y Food_WMAV1_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_WMAV2.mkv -codec copy -y Food_WMAV2_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_ALAC.mkv -codec copy -y Food_ALAC_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_VORBIS.mkv -codec copy -y Food_VORBIS_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_EAC3.mkv -codec copy -y Food_EAC3_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_TRUEHD.mkv -codec copy -y Food_TRUEHD_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_MP2.mkv -codec copy -y Food_MP2_.mkv
ffmpeg -i Food_videosrc.mkv -i Food_PCMS24LE.mkv -codec copy -y Food_PCMS24LE_.mkv


```