# Design

## Layers

```
XPlay::doOpen()

  - streamOpen() --> is_

    - videoq(PacketQueue) & .init
    - pictq(FrameQueue) & .init
    - readThread()
      - avformat_open_input(...)
      - streamComponentOpen(video)
      - pkt = av_read_frame()
      - videoq.put(pkt)

streamComponentOpen(video)
  - avcodec_open2
  - viddec.init() & start()

viddec.start()
  - videoThread()
    - frame = getVideoFrame()
      - viddec.decodeFrame(frame)
        - avcodec_receive_frame(frame)
        - queue(videoq).get(pkt)
        - avcodec_send_packet(pkt)
    - queuePicture(frame)
```