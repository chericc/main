#ifndef __WAV_DEF_H__
#define __WAV_DEF_H__

#include <stdio.h>

#ifndef __cplusplus
extern "C" {
#endif // __cplusplus

/* Ref: wav/fmt: ffmpeg/libavformat/riff.c:ff_codec_wav_tags */
enum WAV_AUDIO_TYPE {
    WAV_AUDIO_TYPE_PCM = 1,
    WAV_AUDIO_TYPE_ALAW = 6,
    WAV_AUDIO_TYPE_MULAW = 7,
};

struct wav_info {

    enum WAV_AUDIO_TYPE audio_type;
    int channels; // 1 
    int sample_rate; // 44100, ...
    int bits_per_sample; // 16, ...
};

struct wav_muxer_info {
    FILE *fp;
    struct wav_info info;
};

struct wav_demuxer_info {
    FILE *fp;
};

#ifndef __cplusplus
}
#endif // __cplusplus

#endif // __WAV_DEF_H__