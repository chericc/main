#ifndef __WAV_MUXER_HPP__
#define __WAV_MUXER_HPP__

#include "wav_def.h"

#ifndef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void* wav_demuxer_handle;
#ifdef __cplusplus
#define wav_demuxer_handle_invalid (nullptr)
#else
#define wav_demuxer_handle_invalid (NULL)
#endif

wav_demuxer_handle wav_demuxer_create(struct wav_demuxer_info const *info);
int wav_demuxer_get_info(struct wav_info *info);
int wav_demuxer_get_data(size_t offset, size_t size, void *output_data, size_t output_data_size);
int wav_demuxer_close(wav_demuxer_handle handle);

#ifndef __cplusplus
}
#endif // __cplusplus

#endif // __WAV_MUXER_HPP__