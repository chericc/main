# ffmpeg

## encode

```

ffmpeg -hwaccel:v vaapi -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 2400 -c:v hevc -r 24 -b:v 256k -b:a 16k   output.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -vf scale=640:-1 -frames:v 1000 -c:v hevc -r 25 -b:v 128k -b:a 32k  -c:a ac3 -ac 1 output_hevc_ac3.mkv

full:
ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -vf scale=640:-1 -c:v hevc -r 25 -b:v 256k -b:a 64k  -c:a aac -ac 1 -q:v 1 output.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v hevc -r 25 -b:v 480k -b:a 96k  -c:a aac -ac 1 -q:v 1 output_ori.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v hevc -crf 28 -c:a aac -b:a 64k -map 0:0 -map 0:1 -ac 1 output_hevc_crf28.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v hevc -vf scale=640:-1 -crf 28 -c:a aac -b:a 64k -map 0:0 -map 0:1 -ac 1 output_hevc_crf28_640.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -c:v hevc -vf scale=640:-1 -crf 28 -c:a aac -b:a 64k -map 0:0 -map 0:1 -ac 1 output_hevc_crf28_640.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -c:v hevc -crf 32 -c:a aac -b:a 96k -map 0:0 -map 0:1 -ac 1 output_hevc_crf32_1920.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v hevc -crf 45 -c:a aac -b:a 96k -map 0:0 -map 0:1 -ac 1 output_hevc_crf45_1920.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v vp9 -crf 45 -c:a aac -b:a 96k -map 0:0 -map 0:1 -ac 1 output_vp9_crf45_1920.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v vp9 -crf 60 -b:v 0 -c:a aac -b:a 96k -map 0:0 -map 0:1 -ac 1 output_vp9_crf60_1920.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v vp9 -crf 60 -b:v 0 -c:a aac -b:a 96k -map 0:0 -map 0:1 -pass 1 -an -f null /dev/null &&\
ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v vp9 -crf 60 -b:v 0 -c:a aac -b:a 96k -map 0:0 -map 0:1 -pass 2 -ac 1 output_vp9_crf60_1920_2pass.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -frames:v 1000 -c:v hevc_nvenc -crf 45 -c:a aac -b:a 96k -map 0:0 -map 0:1 -ac 1 output_hevc_crf45_1920.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -c:v hevc_nvenc -preset p7 -2pass -c:a aac -b:a 256k -map 0:0 -map 0:1 -ac 1 output_hevc_crf45_1920.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -c:v hevc_nvenc -preset p7 -rc-lookahead 25 -cq 28 -c:a aac -b:a 256k -map 0:0 -map 0:1 -ac 1 output_hevc_crf45_1920.mkv

ffmpeg -i '[SAIO-Raws] 千と千尋の神隠し Spirited Away [BD 1920x1036 HEVC-10bit OPUSx2 AC3 (chs&cht,Jpn)].mkv' -c:v hevc_nvenc -preset p7 -rc-lookahead 25 -cq 32 -map 0:0 -map 0:1 -c:a aac -b:a 128k -map 0:4 -map 0:5 -map 0:6 -map 0:7 converted/output_hevc_cq32_1920.mkv

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -c:v hevc_nvenc -preset p7 -rc-lookahead 25 -cq 32 -map 0:0 -map 0:1 -c:a aac -b:a 128k "converted/功夫熊猫.Kung.Fu.Panda.2008.1080P.HEVC.ACC.mkv"

ffmpeg -i qianxun.mkv -to 06:30 -c:v libx264 -c:a copy -map 0:0 -map 0:1 -vf scale=640:320 -ss 06:00 output_qianxun_640_360.mkv

```

## 提取流

ffmpeg -i test.mp4 -vcodec copy -f h264 test.h264

ffmpeg -i test.mp4 -vcodec copy -f h265 test.h265

ffmpeg -i test.mp4 -vcodec copy test.h265

## 获取信息

ffprobe -show_streams -show_pixel_formats -show_packets -show_frames -of xml -count_frames -count_packets -show_pixel_formats


-select_streams

ffprobe -select_streams v:0 -pretty -show_packets

ffprobe -select_streams v:0 -pretty -show_frames


## 指定长度

ffmpeg -i test.h265 -vcodec copy -ss 16 -t 20 test_cut.h265

ffmpeg -i test.h265 -vcodec copy -ss 15:47 -to 16:26 test_cut.h265

ffmpeg -i test.h265 -vcodec copy test_cut.h265

ffmpeg -i ipc00000.ps -vcodec copy -ss 00:49 -to 02:47 1.mp4
ffmpeg -i ipc00000.ps -vcodec copy -ss 04:30 -to 06:05 2.mp4
ffmpeg -i ipc00000.ps -vcodec copy -ss 06:35 -to 08:25 3.mp4
ffmpeg -i ipc00000.ps -vcodec copy -acodec copy -t 09:00 total.mp4

## 不提取音频

ffmpeg -i total.mp4 -an -ss 00:56 -t 30 -vcodec copy no.mp4

## x265 配置

ffmpeg -i sample.mp4 -c:v libx265 -an -x265-params bitrate=1300:no-scenecut=0:keyint=30:min-keyint=30:bframes=0:b-adapt=0 output.mp4

## yuv 编码

YUV420

ffmpeg -pix_fmt nv21 -s 640*360 -i output.yuv -r 20 output.mp4

## av1

声道：-ac:a
采样率：-ar 48000

ffmpeg -i local/content/video/High.School.Musical.2006.BluRay.720p.x264.2Audio.AC3-CnSCG.mkv -c:v libaom-av1 -b:v 100k -c:a aac -b:a 32k -ac:a 1 -cpu-used 8 -frames:v 1000 output_av1_100.mp4

ffmpeg -i local/content/video/High.School.Musical.2006.BluRay.720p.x264.2Audio.AC3-CnSCG.mkv -c:v libaom-av1 -b:v 100k -c:a aac -b:a 16k -ac:a 1 -ar 16000 -cpu-used 8 -frames:v 200 -ss 01:00 output_av1_100.mp4
219KB

ffmpeg -i local/content/video/High.School.Musical.2006.BluRay.720p.x264.2Audio.AC3-CnSCG.mkv -c:v libaom-av1 -crf 32 -c:a aac -b:a 16k -ac:a 1 -ar 16000 -cpu-used 8 -frames:v 200 -ss 01:00 output_av1_100.mp4
752KB

ffmpeg -i local/content/video/High.School.Musical.2006.BluRay.720p.x264.2Audio.AC3-CnSCG.mkv -c:v libaom-av1 -crf 46 -c:a aac -b:a 16k -ac:a 1 -ar 16000 -cpu-used 8 -frames:v 200 -ss 01:00 output_av1_100.mp4
322KB

ffmpeg -i local/content/video/High.School.Musical.2006.BluRay.720p.x264.2Audio.AC3-CnSCG.mkv -c:v libaom-av1 -crf 52 -c:a aac -b:a 16k -ac:a 1 -ar 16000 -cpu-used 8 -frames:v 200 -ss 01:00 output_av1_100.mp4
235

ffmpeg -i local/content/video/High.School.Musical.2006.BluRay.720p.x264.2Audio.AC3-CnSCG.mkv -c:v libaom-av1 -crf 60 -c:a aac -b:a 16k -ac:a 1 -ar 16000 -cpu-used 8 -frames:v 200 -ss 01:00 output_av1_100.mp4
144KB

final:

ffmpeg -i local/content/video/High.School.Musical.2006.BluRay.720p.x264.2Audio.AC3-CnSCG.mkv -c:v libaom-av1 -crf 52 -c:a aac -b:a 16k -ac:a 1 -ar 16000 -cpu-used 8 output_av1_100.mp4

ffmpeg -i ironman_orig.mkv -an -c:v libaom-av1 -b:v 64k -an -cpu-used 8 output_av1_64k_ironman.mp4
ffmpeg -i ironman_orig.mkv -an -c:v libaom-av1 -b:v 256k -an -cpu-used 8 output_av1_256k_ironman.mp4
ffmpeg -i ironman_orig.mkv -vn -c:a libopus -map 0:1 -ab 32k -ac 2 -ar 48000 output_opus_32k_ironman.mp4
ffmpeg -i ironman_orig.mkv -vn -c:a libopus -map 0:1 -ab 16k -ac 2 -ar 48000 output_opus_16k_ironman.mp4
ffmpeg -i output_av1_64k_ironman.mp4 -i output_opus_32k_ironman.mp4 -i iron_en.srt -c:v copy -c:a copy output_ironman_64k_av1_32k_opus.mkv
ffmpeg -i output_av1_64k_ironman.mp4 -i output_opus_16k_ironman.mp4 -i iron_en.srt -c:v copy -c:a copy output_ironman_64k_av1_16k_opus.mkv
ffmpeg -i output_av1_256k_ironman.mp4 -i output_opus_32k_ironman.mp4 -i iron_en.srt -c:v copy -c:a copy output_ironman_256k_av1_32k_opus.mkv

ffmpeg -i ~/local/content/video/功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin\&English.CHS-ENG.Adans.mkv -c:v libaom-av1 -b:v 64k -an -cpu-used 16 output_av1_64k_panda_total.mp4

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv'-c:v libaom-av1 -b:v 64k -an -cpu-used 8 output_av1_64k_panda_total.mp4

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv'-c:v libsvtav1 -b:v 64k -an -cpu-used 16 output_av1_64k_panda_total.mp4

ffmpeg -i musical.orig.mkv -an -c:v libaom-av1 -b:v 512k -an -cpu-used 4 output_av1_512k_musical.mp4

## aac

ffmpeg -i '功夫熊猫.Kung.Fu.Panda.2008.BD1080P.X264.AC3.Mandarin&English.CHS-ENG.Adans.mkv' -vn -c:a aac -b:a 8k  -ar 44100  -cpu-used 8 -ac 1 output_aac.aac

## 提取关键帧

ffmpeg -i test.mp4 -vf select='eq(pict_type\,I)' -vsync 2 -f image2 kf-%02d.bmp