#pragma once

#include <assert.h>

enum AudioFileCodec
{
    ACODEC_UNKNOWN = 0,
    ACODEC_PCM,
    ACODEC_G711A,
    ACODEC_G711U,
    ACODEC_AAC,
};

#ifdef ENABLE_ASSERT
#define AUDIO_ASSERT(x) assert(x)
#else 
#define AUDIO_ASSERT(x) do{x;}while (0)
#endif 
