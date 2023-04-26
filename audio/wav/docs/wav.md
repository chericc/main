# wav

## ref

```bash
http://soundfile.sapp.org/doc/WaveFormat/
ffmpeg/libavformat/wavdec.c/ff_wav_demuxer

wav/fmt: ffmpeg/libavformat/riff.c:ff_codec_wav_tags

```

## header

```bash
wav头：

RIFF + FMT + [FACT] + [LIST] + data

RIFF(4) + RIFF len(4) + WAVE(4) +
fmt(4) + fmt subchunk size(4) + fmt content +
FACT(4) + FACT subchunk size(4) + FACT content +
LIST(4) + LIST subchunk size(4) + LIST content +
data(4) + data subchunk size(4) + data content

RIFF:(8+4=12) 
ChunkID(4)
ChunkSize(4)
Format(4)

fmt(8+16=24): 
SubChunk1ID(4)
Subchunk1Size(4)
AudioFormat(2)
NumChannels(2)
SampleRate(4)
ByteRate(4)
BlockAlign(2)
BitsPerSample(2)

data(8+x):
Subchunk2ID(4)
Subchunk2Size(4)
...
```

## ffmpeg/read_header

```bash
最小字节数：
>44
```

### ffmpeg/wav/fmt

```C

const AVCodecTag ff_codec_wav_tags[] = {
    { AV_CODEC_ID_PCM_S16LE,       0x0001 },
    /* must come after s16le in this list */
    { AV_CODEC_ID_PCM_U8,          0x0001 },
    { AV_CODEC_ID_PCM_S24LE,       0x0001 },
    { AV_CODEC_ID_PCM_S32LE,       0x0001 },
    { AV_CODEC_ID_PCM_S64LE,       0x0001 },
    { AV_CODEC_ID_ADPCM_MS,        0x0002 },
    { AV_CODEC_ID_PCM_F32LE,       0x0003 },
    /* must come after f32le in this list */
    { AV_CODEC_ID_PCM_F64LE,       0x0003 },
    { AV_CODEC_ID_PCM_ALAW,        0x0006 },
    { AV_CODEC_ID_PCM_MULAW,       0x0007 },
    { AV_CODEC_ID_WMAVOICE,        0x000A },
    { AV_CODEC_ID_ADPCM_IMA_OKI,   0x0010 },
    { AV_CODEC_ID_ADPCM_IMA_WAV,   0x0011 },
    /* must come after adpcm_ima_wav in this list */
    { AV_CODEC_ID_ADPCM_ZORK,      0x0011 },
    { AV_CODEC_ID_ADPCM_IMA_OKI,   0x0017 },
    { AV_CODEC_ID_ADPCM_YAMAHA,    0x0020 },
    { AV_CODEC_ID_TRUESPEECH,      0x0022 },
    { AV_CODEC_ID_GSM_MS,          0x0031 },
    { AV_CODEC_ID_GSM_MS,          0x0032 },  /* msn audio */
    { AV_CODEC_ID_AMR_NB,          0x0038 },  /* rogue format number */
    { AV_CODEC_ID_G723_1,          0x0042 },
    { AV_CODEC_ID_ADPCM_G726,      0x0045 },
    { AV_CODEC_ID_ADPCM_G726,      0x0014 },  /* g723 Antex */
    { AV_CODEC_ID_ADPCM_G726,      0x0040 },  /* g721 Antex */
    { AV_CODEC_ID_MP2,             0x0050 },
    { AV_CODEC_ID_MP3,             0x0055 },
    { AV_CODEC_ID_AMR_NB,          0x0057 },
    { AV_CODEC_ID_AMR_WB,          0x0058 },
    /* rogue format number */
    { AV_CODEC_ID_ADPCM_IMA_DK4,   0x0061 },
    /* rogue format number */
    { AV_CODEC_ID_ADPCM_IMA_DK3,   0x0062 },
    { AV_CODEC_ID_ADPCM_G726,      0x0064 },
    { AV_CODEC_ID_ADPCM_IMA_WAV,   0x0069 },
    { AV_CODEC_ID_METASOUND,       0x0075 },
    { AV_CODEC_ID_G729,            0x0083 },
    { AV_CODEC_ID_AAC,             0x00ff },
    { AV_CODEC_ID_G723_1,          0x0111 },
    { AV_CODEC_ID_SIPR,            0x0130 },
    { AV_CODEC_ID_ACELP_KELVIN,    0x0135 },
    { AV_CODEC_ID_WMAV1,           0x0160 },
    { AV_CODEC_ID_WMAV2,           0x0161 },
    { AV_CODEC_ID_WMAPRO,          0x0162 },
    { AV_CODEC_ID_WMALOSSLESS,     0x0163 },
    { AV_CODEC_ID_XMA1,            0x0165 },
    { AV_CODEC_ID_XMA2,            0x0166 },
    { AV_CODEC_ID_FTR,             0x0180 },
    { AV_CODEC_ID_ADPCM_CT,        0x0200 },
    { AV_CODEC_ID_DVAUDIO,         0x0215 },
    { AV_CODEC_ID_DVAUDIO,         0x0216 },
    { AV_CODEC_ID_ATRAC3,          0x0270 },
    { AV_CODEC_ID_MSNSIREN,        0x028E },
    { AV_CODEC_ID_ADPCM_G722,      0x028F },
    { AV_CODEC_ID_MISC4,           0x0350 },
    { AV_CODEC_ID_IMC,             0x0401 },
    { AV_CODEC_ID_IAC,             0x0402 },
    { AV_CODEC_ID_ON2AVC,          0x0500 },
    { AV_CODEC_ID_ON2AVC,          0x0501 },
    { AV_CODEC_ID_GSM_MS,          0x1500 },
    { AV_CODEC_ID_TRUESPEECH,      0x1501 },
    /* ADTS AAC */
    { AV_CODEC_ID_AAC,             0x1600 },
    { AV_CODEC_ID_AAC_LATM,        0x1602 },
    { AV_CODEC_ID_AC3,             0x2000 },
    /* There is no Microsoft Format Tag for E-AC3, the GUID has to be used */
    { AV_CODEC_ID_EAC3,            0x2000 },
    { AV_CODEC_ID_DTS,             0x2001 },
    { AV_CODEC_ID_SONIC,           0x2048 },
    { AV_CODEC_ID_SONIC_LS,        0x2048 },
    { AV_CODEC_ID_G729,            0x2222 },
    { AV_CODEC_ID_PCM_MULAW,       0x6c75 },
    { AV_CODEC_ID_AAC,             0x706d },
    { AV_CODEC_ID_AAC,             0x4143 },
    { AV_CODEC_ID_FTR,             0x4180 },
    { AV_CODEC_ID_XAN_DPCM,        0x594a },
    { AV_CODEC_ID_G729,            0x729A },
    { AV_CODEC_ID_FTR,             0x8180 },
    { AV_CODEC_ID_G723_1,          0xA100 }, /* Comverse Infosys Ltd. G723 1 */
    { AV_CODEC_ID_AAC,             0xA106 },
    { AV_CODEC_ID_SPEEX,           0xA109 },
    { AV_CODEC_ID_FLAC,            0xF1AC },
    /* DFPWM does not have an assigned format tag; it uses a GUID in WAVEFORMATEX instead */
    { AV_CODEC_ID_DFPWM,           0xFFFE },
    { AV_CODEC_ID_ADPCM_SWF,       ('S' << 8) + 'F' },
    /* HACK/FIXME: Does Vorbis in WAV/AVI have an (in)official ID? */
    { AV_CODEC_ID_VORBIS,          ('V' << 8) + 'o' },
    { AV_CODEC_ID_NONE,      0 },
};
```

