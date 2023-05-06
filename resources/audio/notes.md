# notes

## generate raw audio using ffmpeg

```bash
rm *.raw
ffmpeg -i demo_8000hz_1ch.wav -acodec copy -f s16le demo_8000hz_1ch.raw
ffmpeg -i demo_8000hz_1ch_alaw.wav -acodec copy -f alaw demo_8000hz_1ch_alaw.raw
ffmpeg -i demo_8000hz_1ch_ulaw.wav -acodec copy -f mulaw demo_8000hz_1ch_ulaw.raw
ffmpeg -i demo_16000hz_1ch.wav -acodec copy -f s16le demo_16000hz_1ch.raw
ffmpeg -i demo_16000hz_1ch_alaw.wav -acodec copy -f alaw demo_16000hz_1ch_alaw.raw
ffmpeg -i demo_16000hz_1ch_ulaw.wav -acodec copy -f mulaw demo_16000hz_1ch_ulaw.raw
```