#ifndef __WAV_MUXER_HPP__
#define __WAV_MUXER_HPP__

#include <stddef.h>
#include <stdio.h>

#ifndef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void* wav_muxer_handle;
#define wav_muxer_handle_invalid (NULL)

struct wav_muxer_info {
    FILE *fp;
};

wav_muxer_handle wav_muxer_create(struct wav_muxer_info const *info);
int wav_muxer_input(wav_muxer_handle handle, const void *chunk, size_t count);
int wav_muxer_close(wav_muxer_handle handle);

#ifndef __cplusplus
}
#endif // __cplusplus

#endif // __WAV_MUXER_HPP__