#include "rtsp_server.h"

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

#include <thread>
#include <memory>
#include <cstdio>

#include "xlog.h"

namespace {

struct rtsp_server_ctx {
    std::shared_ptr<std::thread> trd;

    struct rtsp_server_param param = {};

    EventLoopWatchVariable loopVariable;
};

ServerMediaSession* createNewSms(UsageEnvironment& env, const char *filename, FILE *fp);

class MyRTSPServer : public RTSPServer {
public:
    static MyRTSPServer* createNew(UsageEnvironment &env, Port port,
        UserAuthenticationDatabase *authDB, 
        unsigned int reclamationSeconds = 10);
protected:
    MyRTSPServer(UsageEnvironment &env, int ipv4Socket, int ipv6Socket, Port port,
        UserAuthenticationDatabase *authDB, unsigned int reclamationSeconds);
    ~MyRTSPServer() override;

protected:
    void lookupServerMediaSession(const char *streamName,
        lookupServerMediaSessionCompletionFunc *completionFunc,
        void *completionClientData,
        Boolean isFirstLoopupInSession) override;
};

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

void MyRTSPServer::lookupServerMediaSession(const char *streamName,
        lookupServerMediaSessionCompletionFunc *completionFunc,
        void *completionClientData,
        Boolean isFirstLoopupInSession)
{
    bool error_flag = false;
    FILE *fp = nullptr;
    ServerMediaSession *sms = getServerMediaSession(streamName);

    do {
        xlog_dbg("opening file %s\n", streamName);

        fp = fopen(streamName, "rb");
        if (!fp) {
            xlog_dbg("failed to open file: <%s>\n", streamName);
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
            sms = createNewSms(envir(), streamName, fp);
            addServerMediaSession(sms);
        }

        if (completionFunc) {
            (*completionFunc)(completionClientData, sms);
        }
    } while (false);

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    if (error_flag) {
        xlog_err("error\n");
    }

    return ;
}

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
        sms = ServerMediaSession::createNew(env, filename, filename, descStr);\
    } while(0)

ServerMediaSession* createNewSms(UsageEnvironment& env, const char *filename, FILE *fp)
{
    // Use the file name extension to determine the type of "ServerMediaSession":
    char const* extension = strrchr(filename, '.');
    if (extension == NULL) {
        return NULL;
    }

    ServerMediaSession* sms = NULL;
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
        Interleaving* interleaving = NULL;
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
        sprintf(indexFileName, "%sx", filename);
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
        while ((smss = creationState.demux->newServerMediaSubsession()) != NULL) {
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
        while ((smss = creationState.demux->newServerMediaSubsession()) != NULL) {
        sms->addSubsession(smss);
        }
    }

    return sms;
}

void rtsp_server_trd(struct rtsp_server_ctx *ctx)
{

    TaskScheduler *scheduler = nullptr;
    UsageEnvironment *env = nullptr;
    UserAuthenticationDatabase *authDB = nullptr;
    RTSPServer *rtspServer = nullptr;

    do {
        scheduler = BasicTaskScheduler::createNew();
        env = BasicUsageEnvironment::createNew(*scheduler);

        xlog_dbg("server: [%s/%s], port=%d\n", 
            ctx->param.username, ctx->param.password,
            ctx->param.port);

        authDB = new UserAuthenticationDatabase;
        authDB->addUserRecord(ctx->param.username, ctx->param.password);

        portNumBits port = ctx->param.port;
        rtspServer = MyRTSPServer::createNew(*env, port, authDB);
    
        if (!rtspServer) {
            xlog_err("create rtsp server failed\n");
            break;
        }

        xlog_dbg("rtsp server started\n");
        env->taskScheduler().doEventLoop(&ctx->loopVariable);
        xlog_dbg("rtsp server stopped\n");
    } while (0);

    if (rtspServer) {
        Medium::close(rtspServer);
        rtspServer = nullptr;
    }

    if (authDB) {
        delete authDB;
        authDB = nullptr;
    }

    if (env) {
        if (!env->reclaim()) {
            xlog_err("delete env failed\n");
        }
    }

    if (scheduler) {
        delete scheduler;
        scheduler = nullptr;
    }

    return ;
}

}

rtsp_server_obj rtsp_server_new(const struct rtsp_server_param *param)
{
    struct rtsp_server_ctx *ctx = nullptr;

    do {
        ctx = new rtsp_server_ctx{};
        ctx->param = *param;
        ctx->loopVariable = 0;
        ctx->trd = std::make_shared<std::thread>(rtsp_server_trd, ctx);
    } while (0);

    return reinterpret_cast<rtsp_server_obj>(ctx);
}

int rtsp_server_delete(rtsp_server_obj obj)
{
    auto ctx = reinterpret_cast<struct rtsp_server_ctx*>(obj);
    do {
        if (!ctx) {
            break;
        }

        xlog_dbg("notify thread exit\n");
        ctx->loopVariable = 1;

        if (ctx->trd->joinable()) {
            xlog_dbg("wait thread\n");
            ctx->trd->join();
            xlog_dbg("wait thread end\n");
        }

        delete ctx;
        ctx = nullptr;
    } while(0);

    return 0;
}