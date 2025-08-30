#ifndef __WAV_MUXER_HPP__
#define __WAV_MUXER_HPP__

#include "wav_def.h"

#ifndef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void* wav_muxer_handle;
#ifdef __cplusplus
#define wav_muxer_handle_invalid (nullptr)
#else
#define wav_muxer_handle_invalid (NULL)
#endif

wav_muxer_handle wav_muxer_create(struct wav_muxer_info const *info);
int wav_muxer_input(wav_muxer_handle handle, const void *chunk, size_t chunksize, size_t count);
int wav_muxer_close(wav_muxer_handle handle);

#ifndef __cplusplus
}
#endif // __cplusplus

#endif // __WAV_MUXER_HPP__