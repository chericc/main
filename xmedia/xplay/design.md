# Design

## Layers

```
XPlay::doOpen()

  - streamOpen() --> is_

    - videoq(PacketQueue) & .init
    - pictq(FrameQueue) & .init
    - readThread()

      - streamComponentOpen(video)

      - pkt = av_read_frame()
      - videoq.put(pkt)

streamComponentOpen(video)
  - 
```