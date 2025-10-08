# live555

## faq

http://www.live555.com/liveMedia/faq.html

## live555

### live-server

```
参考file-server的实现，需要继承OnDemandServerMediaSubsession实现一个live sub session。

其中，createNewStreamSource需要实现一个FramedSource。参考已有的ByteStreamFileSource实现。
```

### refer

playCommon.cpp

### MediaSink::continuePlaying

```bash
source(fReadSource) --> getNextFrame()

--> FramedSource.cpp:: sub_source.doGetNextFrame()

```

### fReadSource

```bash
ms = MediaSession(sdp)

ms --> MediaSubsessionIterator

MediaSubsessionIterator --> MediaSubsession

sub->initiate() --> 
MediaSesson.cpp:: sub->createSourceObjects() --> fReadSource = createNew()

sink --> startPlaying(sub->readSource() --> fReadSource)
--> sink --> continuePlaying
--> sink --> getNextFrame() -later-> afterGettingFrame() --> continuePlaying


```

### H264VideoRTPSource

```bash
MultiFrameRTPSource::doGetNextFrame()

--> MultiFrameRTPSource::doGetNextFrame1()

--> fRecorderingBuffer->getNextCompletedPacket()
```