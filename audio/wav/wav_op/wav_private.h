#ifndef __WAV_PRIVATE_H__
#define __WAV_PRIVATE_H__

#define MKTAG(a, b, c, d) \
    ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

#endif // __WAV_PRIVATE_H__