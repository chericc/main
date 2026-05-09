# RTSP File Server Design

## Overview

An RTSP file server that streams raw H.264/H.265 video files over RTP. Supports both RTP over TCP (interleaved) and RTP over UDP (unicast) transport.

## Architecture

```
┌─────────────────────────────────────────────────┐
│  main.cpp                                       │
│  ┌───────────────────────────────────────────┐  │
│  │ RTSP Callbacks (ondescribe, onsetup, etc) │  │
│  └──────────┬────────────────────────────────┘  │
│             │ uses                              │
│  ┌──────────▼──────────────────────────────┐    │
│  │ IMediaSource interface                   │    │
│  │  ├── H264FileSource (.h264 files)        │    │
│  │  └── H265FileSource (.h265 files)        │    │
│  └──────────┬──────────────────────────────┘    │
│             │ uses                              │
│  ┌──────────▼──────────────────────────────┐    │
│  │ IRTPTransport interface                  │    │
│  │  ├── RTPTcpTransport (interleaved TCP)   │    │
│  │  └── RTPUdpTransport (UDP unicast)       │    │
│  └─────────────────────────────────────────┘    │
└─────────────────────────────────────────────────┘
```

## Key Files

| File | Purpose |
|---|---|
| `main.cpp` | RTSP server entry point, callback handlers, session management |
| `media-source.h` | `IMediaSource` and `IRTPTransport` abstract interfaces |
| `h264-file-reader.h/.cpp` | Raw `.h264` annex B file parser (nalu indexing, SPS/PPS extraction) |
| `h264-file-source.h/.cpp` | `IMediaSource` impl: reads H264 frames, packetizes into RTP |
| `h265-file-reader.h/.cpp` | Raw `.h265` annex B file parser |
| `h265-file-source.h/.cpp` | `IMediaSource` impl for H265 |
| `rtp-tcp-transport.h` | `IRTPTransport` impl: sends RTP over RTSP interleaved TCP |
| `rtp-udp-transport.h/.cpp` | `IRTPTransport` impl: sends RTP over UDP unicast |

## Source File Dependencies

All source files depend only on core libraries within the project (`librtsp`, `librtp`, `libbase`, `libaio`). No external dependencies.

| File | Dependencies |
|---|---|
| `h264-file-reader` | Standard C++ + `ctypedef.h` (libbase) |
| `h264-file-source` | reader + `media-source.h` + librtp API + libbase |
| `h265-file-reader` | Standard C++ + `ctypedef.h` (libbase) |
| `h265-file-source` | reader + `media-source.h` + librtp API + libbase |
| `rtp-tcp-transport` | `rtsp-server.h` (librtsp) + `media-source.h` |
| `rtp-udp-transport` | `media-source.h` + libbase network utilities |

## RTSP Callback Flow

```
1. DESCRIBE  → rtsp_ondescribe
   - Parse URI, extract filename (/vod/ prefix)
   - Create IMediaSource for .h264/.h265 file
   - Build SDP (duration, codec params, SPS/PPS base64)
   - Cache result in s_describes map
   - Reply with SDP

2. SETUP     → rtsp_onsetup
   - Parse URI, extract filename
   - Create IMediaSource if new session
   - Negotiate transport (TCP interleaved or UDP unicast)
   - Create IRTPTransport, attach to IMediaSource
   - Reply with session ID and transport info

3. PLAY      → rtsp_onplay
   - Look up session
   - Handle optional seek (NPT) and speed
   - Get RTP info (seq, timestamp)
   - Set session status to playing
   - Reply with RTP-Info

4. PAUSE     → rtsp_onpause
   - Look up session
   - Pause media source
   - Set session status to paused

5. TEARDOWN  → rtsp_onteardown
   - Look up session
   - Remove from sessions map
   - Reply OK

6. onerror   → Clean up session associated with errored RTSP handle
```

## Main Loop

After a keypress, the server enters a polling loop:
- Every 5 seconds, iterate all active sessions
- For sessions in PLAYING state, call `media->Play()` to send one frame
- Play() paces itself (checks `m_rtp_clock + 40 < now`), so each call sends at most 1 frame at 25fps timing

## Usage

```bash
# Start server
./demo_my_rtsp_server
# Press Enter to start serving

# Client (ffplay)
ffplay rtsp://127.0.0.1:8554/vod/test.h264
```
