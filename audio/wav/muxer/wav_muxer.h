#ifndef __WAV_MUXER_HPP__
#define __WAV_MUXER_HPP__

#include <stddef.h>
#include <stdio.h>

#ifndef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void* wav_muxer_handle;
#ifdef __cplusplus
#define wav_muxer_handle_invalid (nullptr)
#else
#define wav_muxer_handle_invalid (NULL)
#endif 

/* Ref: wav/fmt: ffmpeg/libavformat/riff.c:ff_codec_wav_tags */
enum WAV_AUDIO_TYPE {
    WAV_AUDIO_TYPE_PCM = 1,
};

struct wav_muxer_info {
    FILE *fp;

    enum WAV_AUDIO_TYPE audio_type;
    int channels; // 1 
    int sample_rate; // 44100, ...
    int bits_per_sample; // 16, ...
};

wav_muxer_handle wav_muxer_create(struct wav_muxer_info const *info);
int wav_muxer_input(wav_muxer_handle handle, const void *chunk, size_t chunksize, size_t count);
int wav_muxer_close(wav_muxer_handle handle);

#ifndef __cplusplus
}
#endif // __cplusplus

#endif // __WAV_MUXER_HPP__