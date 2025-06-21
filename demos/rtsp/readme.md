# live555

## faq

http://www.live555.com/liveMedia/faq.html

## live555

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