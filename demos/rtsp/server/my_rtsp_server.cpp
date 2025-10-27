#include <string>

#include "my_rtsp_server.hpp"
#include "live_media_subsession.hpp"

#include "xlog.h"

namespace {

struct MatroskaDemuxCreationState 
{
    MatroskaFileServerDemux* demux;
    EventLoopWatchVariable watchVariable;
};

void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux, void* clientData) 
{
    MatroskaDemuxCreationState* creationState = (MatroskaDemuxCreationState*)clientData;
    creationState->demux = newDemux;
    creationState->watchVariable = 1;
}

struct OggDemuxCreationState 
{
    OggFileServerDemux* demux;
    EventLoopWatchVariable watchVariable;
};

void onOggDemuxCreation(OggFileServerDemux* newDemux, void* clientData) 
{
    OggDemuxCreationState* creationState = (OggDemuxCreationState*)clientData;
    creationState->demux = newDemux;
    creationState->watchVariable = 1;
}
// END Special code for handling Ogg files:

#define NEW_SMS(description) \
    do {\
        char const* descStr = description\
            ", streamed by the LIVE555 Media Server";\
        sms = ServerMediaSession::createNew(env, streamname, filename, descStr);\
    } while(0)

ServerMediaSession* createLiveSms(UsageEnvironment& env, const char *filename, const char *streamname)
{
    ServerMediaSession *sms = nullptr;

    do {
        OutPacketBuffer::maxSize = 512 * 1024;
        RTPSink *sink = nullptr;

        // sink = H264VideoRTPSink::createNew(env, );

        sms = ServerMediaSession::createNew(env, streamname, streamname, 
            "live stream", True);
        // sms->addSubsession(PassiveServerMediaSubsession::createNew())
    } while (false);
}

ServerMediaSession* createNewFileSms(UsageEnvironment& env, const char *filename, const char *streamname)
{
    // Use the file name extension to determine the type of "ServerMediaSession":
    char const* extension = strrchr(filename, '.');
    if (extension == nullptr) {
        return nullptr;
    }

    ServerMediaSession* sms = nullptr;
    Boolean const reuseSource = False;
    if (strcmp(extension, ".aac") == 0) {
        // Assumed to be an AAC Audio (ADTS format) file:
        NEW_SMS("AAC Audio");
        sms->addSubsession(ADTSAudioFileServerMediaSubsession::createNew(env, filename, reuseSource));
    } else if (strcmp(extension, ".amr") == 0) {
        // Assumed to be an AMR Audio file:
        NEW_SMS("AMR Audio");
        sms->addSubsession(AMRAudioFileServerMediaSubsession::createNew(env, filename, reuseSource));
    } else if (strcmp(extension, ".ac3") == 0) {
        // Assumed to be an AC-3 Audio file:
        NEW_SMS("AC-3 Audio");
        sms->addSubsession(AC3AudioFileServerMediaSubsession::createNew(env, filename, reuseSource));
    } else if (strcmp(extension, ".m4e") == 0) {
        // Assumed to be a MPEG-4 Video Elementary Stream file:
        NEW_SMS("MPEG-4 Video");
        sms->addSubsession(MPEG4VideoFileServerMediaSubsession::createNew(env, filename, reuseSource));
    } else if (strcmp(extension, ".264") == 0) {
        // Assumed to be a H.264 Video Elementary Stream file:
        NEW_SMS("H.264 Video");
        OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.264 frames
        sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(env, filename, reuseSource));
    } else if (strcmp(extension, ".265") == 0) {
        // Assumed to be a H.265 Video Elementary Stream file:
        NEW_SMS("H.265 Video");
        OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.265 frames
        sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(env, filename, reuseSource));
    } else if (strcmp(extension, ".mp3") == 0) {
        // Assumed to be a MPEG-1 or 2 Audio file:
        NEW_SMS("MPEG-1 or 2 Audio");
        // To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
    //#define STREAM_USING_ADUS 1
        // To also reorder ADUs before streaming, uncomment the following:
    //#define INTERLEAVE_ADUS 1
        // (For more information about ADUs and interleaving,
        //  see <http://www.live555.com/rtp-mp3/>)
        Boolean useADUs = False;
        Interleaving* interleaving = nullptr;
    #ifdef STREAM_USING_ADUS
        useADUs = True;
    #ifdef INTERLEAVE_ADUS
        unsigned char interleaveCycle[] = {0,2,1,3}; // or choose your own...
        unsigned const interleaveCycleSize
        = (sizeof interleaveCycle)/(sizeof (unsigned char));
        interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
    #endif
    #endif
        sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(env, filename, reuseSource, useADUs, interleaving));
    } else if (strcmp(extension, ".mpg") == 0) {
        // Assumed to be a MPEG-1 or 2 Program Stream (audio+video) file:
        NEW_SMS("MPEG-1 or 2 Program Stream");
        MPEG1or2FileServerDemux* demux = MPEG1or2FileServerDemux::createNew(env, filename, reuseSource);
        sms->addSubsession(demux->newVideoServerMediaSubsession());
        sms->addSubsession(demux->newAudioServerMediaSubsession());
    } else if (strcmp(extension, ".vob") == 0) {
        // Assumed to be a VOB (MPEG-2 Program Stream, with AC-3 audio) file:
        NEW_SMS("VOB (MPEG-2 video with AC-3 audio)");
        MPEG1or2FileServerDemux* demux
        = MPEG1or2FileServerDemux::createNew(env, filename, reuseSource);
        sms->addSubsession(demux->newVideoServerMediaSubsession());
        sms->addSubsession(demux->newAC3AudioServerMediaSubsession());
    } else if (strcmp(extension, ".ts") == 0) {
        // Assumed to be a MPEG Transport Stream file:
        // Use an index file name that's the same as the TS file name, except with ".tsx":
        unsigned indexFileNameLen = strlen(filename) + 2; // allow for trailing "x\0"
        char* indexFileName = new char[indexFileNameLen];
        snprintf(indexFileName, indexFileNameLen, "%sx", filename);
        NEW_SMS("MPEG Transport Stream");
        sms->addSubsession(MPEG2TransportFileServerMediaSubsession::createNew(env, filename, indexFileName, reuseSource));
        delete[] indexFileName;
    } else if (strcmp(extension, ".wav") == 0) {
        // Assumed to be a WAV Audio file:
        NEW_SMS("WAV Audio Stream");
        // To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
        // change the following to True:
        Boolean convertToULaw = False;
        sms->addSubsession(WAVAudioFileServerMediaSubsession::createNew(env, filename, reuseSource, convertToULaw));
    } else if (strcmp(extension, ".dv") == 0) {
        // Assumed to be a DV Video file
        // First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
        OutPacketBuffer::maxSize = 300000;

        NEW_SMS("DV Video");
        sms->addSubsession(DVVideoFileServerMediaSubsession::createNew(env, filename, reuseSource));
    } else if (strcmp(extension, ".mkv") == 0 || strcmp(extension, ".webm") == 0) {
        // Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
        OutPacketBuffer::maxSize = 300000; // allow for some possibly large VP8 or VP9 frames
        NEW_SMS("Matroska video+audio+(optional)subtitles");

        // Create a Matroska file server demultiplexor for the specified file.
        // (We enter the event loop to wait for this to complete.)
        MatroskaDemuxCreationState creationState;
        creationState.watchVariable = 0;
        MatroskaFileServerDemux::createNew(env, filename, onMatroskaDemuxCreation, &creationState);
        env.taskScheduler().doEventLoop(&creationState.watchVariable);

        ServerMediaSubsession* smss;
        while ((smss = creationState.demux->newServerMediaSubsession()) != nullptr) {
            sms->addSubsession(smss);
        }
    } else if (strcmp(extension, ".ogg") == 0 || strcmp(extension, ".ogv") == 0 || strcmp(extension, ".opus") == 0) {
        // Assumed to be an Ogg file
        NEW_SMS("Ogg video and/or audio");

        // Create a Ogg file server demultiplexor for the specified file.
        // (We enter the event loop to wait for this to complete.)
        OggDemuxCreationState creationState;
        creationState.watchVariable = 0;
        OggFileServerDemux::createNew(env, filename, onOggDemuxCreation, &creationState);
        env.taskScheduler().doEventLoop(&creationState.watchVariable);

        ServerMediaSubsession* smss;
        while ((smss = creationState.demux->newServerMediaSubsession()) != nullptr) {
        sms->addSubsession(smss);
        }
    } else {
        xlog_err("file not support: %s\n", filename);
    }

    return sms;
}

}

MyRTSPServer *MyRTSPServer::createNew(UsageEnvironment &env, Port port,
        UserAuthenticationDatabase *authDB, 
        unsigned int reclamationSeconds)
{
    int ourSocketIPv4 = setUpOurSocket(env, port, AF_INET);
    int ourSocketIPv6 = setUpOurSocket(env, port, AF_INET6);
    if (ourSocketIPv4 < 0 && ourSocketIPv6 < 0) {
        return nullptr;
    }

    auto serv = new MyRTSPServer(env, ourSocketIPv4, ourSocketIPv6, port, 
        authDB, reclamationSeconds);
    return serv;
}

MyRTSPServer::MyRTSPServer(UsageEnvironment &env, int ipv4Socket, int ipv6Socket, Port port,
        UserAuthenticationDatabase *authDB, unsigned int reclamationSeconds)
    : RTSPServer(env, ipv4Socket, ipv6Socket, port, authDB, reclamationSeconds)
{
}

MyRTSPServer::~MyRTSPServer() 
{
}


void MyRTSPServer::onVod(const char *filepath, const char *streamname, 
    lookupServerMediaSessionCompletionFunc *completionFunc,
    void *completionClientData,
    Boolean isFirstLoopupInSession)
{
    xlog_dbg("path/streamname: [%s]/[%s]\n", filepath, streamname);

    bool error_flag = false;
    FILE *fp = nullptr;
    ServerMediaSession *sms = getServerMediaSession(streamname);

    do {
        fp = fopen(filepath, "r");
        if (!fp) {
            xlog_dbg("failed to open file: <%s>\n", filepath);
            if (sms) {
                xlog_dbg("sms exist while file not exist, remove sms\n");
                removeServerMediaSession(sms);
                sms = nullptr;
            }
            error_flag = true;
            break;
        }

        if (sms && isFirstLoopupInSession) {
            removeServerMediaSession(sms);
            sms = nullptr;
        }

        if (!sms) {
            sms = createNewFileSms(envir(), filepath, streamname);
            addServerMediaSession(sms);
        }

        if (completionFunc) {
            (*completionFunc)(completionClientData, sms);
        }
    } while (false);

    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }

    if (error_flag) {
        xlog_err("error\n");
    }

}
void MyRTSPServer::onLive(const char *path, const char *streamname, 
    lookupServerMediaSessionCompletionFunc *completionFunc,
    void *completionClientData,
    Boolean isFirstLoopupInSession)
{
    xlog_dbg("path/streamname: [%s]/[%s]\n", path, streamname);

    ServerMediaSession *sms = getServerMediaSession(streamname);
    do {
        if (sms && isFirstLoopupInSession) {
            removeServerMediaSession(sms);
            sms = nullptr;
        }

        if (sms == nullptr) {
            sms = ServerMediaSession::createNew(envir(), streamname, path, "live");
            sms->addSubsession(LiveMediaSubsession::createNew(envir()));
            addServerMediaSession(sms);
        }

        if (nullptr != completionFunc) {
            (*completionFunc)(completionClientData, sms);
        }
    } while (false);
}

void MyRTSPServer::lookupServerMediaSession(const char *streamName,
        lookupServerMediaSessionCompletionFunc *completionFunc,
        void *completionClientData,
        Boolean isFirstLoopupInSession)
{
    // rtsp://ip/vod/test.mkv
    // rtsp://ip/live/0
    // -->
    // vod/test.mkv
    // live/0

    do {
        xlog_dbg("opening stream %s\n", streamName);

        std::string stream(streamName);
        size_t index_slash = stream.find("/");
        if (index_slash == std::string::npos) {
            xlog_err("stream not support\n");
            break;
        }

        std::string type = std::string(stream, 0, index_slash);
        std::string path = std::string(stream, index_slash + 1);
        if (type == "vod") {
            xlog_dbg("vod\n");
            onVod(path.c_str(), streamName, completionFunc, completionClientData, isFirstLoopupInSession);
        } else if (type == "live") {
            // ignore vod path
            xlog_dbg("live\n");
            onLive(path.c_str(), streamName, completionFunc, completionClientData, isFirstLoopupInSession);
        } else {
            xlog_err("type not support: %s\n", type.c_str());
            break;
        }
    } while (false);

    // return
}