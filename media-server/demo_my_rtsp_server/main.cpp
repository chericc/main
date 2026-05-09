#include <thread>
#include <cstring>
#include <map>
#include <memory>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "rtsp-server.h"
#include "rtsp-server-aio.h"
#include "aio-worker.h"
#include "rtp-profile.h"
#include "rtp.h"

#include "ctypedef.h"
#include "sys/sock.h"
#include "sys/system.h"
#include "sys/path.h"
#include "sys/sync.hpp"
#include "sockutil.h"
#include "cstringext.h"
#include "uri-parse.h"
#include "urlcodec.h"
#include "path.h"
#include "ntp-time.h"

#include "h264-file-source.h"
#include "h265-file-source.h"
#include "rtp-tcp-transport.h"
#include "rtp-udp-transport.h"

#include "xlog.h"

#define N_AIO_THREAD 4

static ThreadLocker s_locker;

struct rtsp_media_t
{
    std::shared_ptr<IMediaSource> media;
    std::shared_ptr<IRTPTransport> transport;
    uint8_t channel;
    int status; // 0=init, 1=play, 2=pause
    rtsp_server_t* rtsp;
};
typedef std::map<std::string, rtsp_media_t> TSessions;
static TSessions s_sessions;

struct TFileDescription
{
    int64_t duration;
    std::string sdpmedia;
};
static std::map<std::string, TFileDescription> s_describes;

static int rtsp_uri_parse(const char* uri, std::string& path)
{
    char path1[256];
    struct uri_t* r = uri_parse(uri, strlen(uri));
    if(!r)
        return -1;

    url_decode(r->path, strlen(r->path), path1, sizeof(path1));
    path = path1;
    uri_free(r);
    return 0;
}

static int rtsp_ondescribe(void* /*ptr*/, rtsp_server_t* rtsp, const char* uri)
{
    static const char* pattern_vod =
        "v=0\n"
        "o=- %llu %llu IN IP4 %s\n"
        "s=%s\n"
        "c=IN IP4 0.0.0.0\n"
        "t=0 0\n"
        "a=range:npt=0-%.1f\n"
        "a=recvonly\n"
        "a=control:*\n";

    std::string filename;
    std::map<std::string, TFileDescription>::const_iterator it;

    rtsp_uri_parse(uri, filename);
    if (strstartswith(filename.c_str(), "/vod/"))
    {
        // strip /vod/ prefix, keep as relative path
        filename = filename.c_str() + 5;
    }
    else
    {
        return rtsp_server_reply_describe(rtsp, 404, NULL);
    }

    char buffer[1024] = { 0 };
    {
        AutoThreadLocker locker(s_locker);
        it = s_describes.find(filename);
        if(it == s_describes.end())
        {
            TFileDescription describe;
            std::shared_ptr<IMediaSource> source;

            if (strendswith(filename.c_str(), ".h264"))
                source.reset(new H264FileSource(filename.c_str()));
            else if (strendswith(filename.c_str(), ".h265"))
                source.reset(new H265FileSource(filename.c_str()));
            else
            {
                // unsupported format
                return rtsp_server_reply_describe(rtsp, 404, NULL);
            }

            source->GetDuration(describe.duration);

            int offset = snprintf(buffer, sizeof(buffer), pattern_vod,
                ntp64_now(), ntp64_now(), "0.0.0.0", uri, describe.duration / 1000.0);
            assert(offset > 0 && offset + 1 < sizeof(buffer));

            source->GetSDPMedia(describe.sdpmedia);

            it = s_describes.insert(std::make_pair(filename, describe)).first;
        }
    }

    std::string sdp = buffer;
    sdp += it->second.sdpmedia;
    return rtsp_server_reply_describe(rtsp, 200, sdp.c_str());
}

static int rtsp_onsetup(void* /*ptr*/, rtsp_server_t* rtsp, const char* uri,
    const char* session, const struct rtsp_header_transport_t transports[], size_t num)
{
    std::string filename;
    char rtsp_transport[128];
    const struct rtsp_header_transport_t *transport = NULL;

    rtsp_uri_parse(uri, filename);
    if (strstartswith(filename.c_str(), "/vod/"))
    {
        filename = filename.c_str() + 5;
    }
    else
    {
        return rtsp_server_reply_setup(rtsp, 404, NULL, NULL);
    }

    if ('\\' == *filename.rbegin() || '/' == *filename.rbegin())
        filename.erase(filename.end() - 1);
    const char* basename = path_basename(filename.c_str());
    if (NULL == strchr(basename, '.')) // filter track1
        filename.erase(basename - filename.c_str() - 1, std::string::npos);

    TSessions::iterator it;
    if(session)
    {
        AutoThreadLocker locker(s_locker);
        it = s_sessions.find(session);
        if(it == s_sessions.end())
        {
            return rtsp_server_reply_setup(rtsp, 454, NULL, NULL);
        }
        // aggregate control not supported
    }
    else
    {
        rtsp_media_t item;
        item.rtsp = rtsp;
        item.channel = 0;
        item.status = 0;

        if (strendswith(filename.c_str(), ".h264"))
            item.media.reset(new H264FileSource(filename.c_str()));
        else if (strendswith(filename.c_str(), ".h265"))
            item.media.reset(new H265FileSource(filename.c_str()));
        else
        {
            return rtsp_server_reply_setup(rtsp, 404, NULL, NULL);
        }

        char rtspsession[32];
        snprintf(rtspsession, sizeof(rtspsession), "%p", item.media.get());

        AutoThreadLocker locker(s_locker);
        it = s_sessions.insert(std::make_pair(rtspsession, item)).first;
    }

    for(size_t i = 0; i < num && !transport; i++)
    {
        if(RTSP_TRANSPORT_RTP_UDP == transports[i].transport)
        {
            transport = &transports[i];
        }
        else if(RTSP_TRANSPORT_RTP_TCP == transports[i].transport)
        {
            transport = &transports[i];
        }
    }
    if(!transport)
    {
        return rtsp_server_reply_setup(rtsp, 461, NULL, NULL);
    }

    rtsp_media_t &item = it->second;
    if (RTSP_TRANSPORT_RTP_TCP == transport->transport)
    {
        int interleaved[2];
        if (transport->interleaved1 == transport->interleaved2)
        {
            interleaved[0] = item.channel++;
            interleaved[1] = item.channel++;
        }
        else
        {
            interleaved[0] = transport->interleaved1;
            interleaved[1] = transport->interleaved2;
        }

        item.transport = std::make_shared<RTPTcpTransport>(rtsp, interleaved[0], interleaved[1]);
        item.media->SetTransport(path_basename(uri), item.transport);

        snprintf(rtsp_transport, sizeof(rtsp_transport),
            "RTP/AVP/TCP;interleaved=%d-%d", interleaved[0], interleaved[1]);
    }
    else
    {
        // unicast
        item.transport = std::make_shared<RTPUdpTransport>();

        assert(transport->rtp.u.client_port1 && transport->rtp.u.client_port2);
        unsigned short port[2] = { transport->rtp.u.client_port1, transport->rtp.u.client_port2 };
        const char *ip = transport->destination[0] ? transport->destination : rtsp_server_get_client(rtsp, NULL);
        if(0 != ((RTPUdpTransport*)item.transport.get())->Init(ip, port))
        {
            return rtsp_server_reply_setup(rtsp, 500, NULL, NULL);
        }
        item.media->SetTransport(path_basename(uri), item.transport);

        snprintf(rtsp_transport, sizeof(rtsp_transport),
            "RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu%s%s",
            transport->rtp.u.client_port1, transport->rtp.u.client_port2,
            port[0], port[1],
            transport->destination[0] ? ";destination=" : "",
            transport->destination[0] ? transport->destination : "");
    }

    return rtsp_server_reply_setup(rtsp, 200, it->first.c_str(), rtsp_transport);
}

static int rtsp_onplay(void* /*ptr*/, rtsp_server_t* rtsp, const char* uri,
    const char* session, const int64_t *npt, const double *scale)
{
    std::shared_ptr<IMediaSource> source;
    TSessions::iterator it;
    {
        AutoThreadLocker locker(s_locker);
        it = s_sessions.find(session ? session : "");
        if(it == s_sessions.end())
        {
            return rtsp_server_reply_play(rtsp, 454, NULL, NULL, NULL);
        }

        source = it->second.media;
    }
    if(npt && 0 != source->Seek(*npt))
    {
        return rtsp_server_reply_play(rtsp, 457, NULL, NULL, NULL);
    }

    if(scale && 0 != source->SetSpeed(*scale))
    {
        assert(*scale > 0);
        return rtsp_server_reply_play(rtsp, 406, NULL, NULL, NULL);
    }

    char rtpinfo[512] = { 0 };
    source->GetRTPInfo(uri, rtpinfo, sizeof(rtpinfo));

    it->second.status = 1;
    return rtsp_server_reply_play(rtsp, 200, npt, NULL, rtpinfo);
}

static int rtsp_onpause(void* /*ptr*/, rtsp_server_t* rtsp, const char* /*uri*/,
    const char* session, const int64_t* /*npt*/)
{
    std::shared_ptr<IMediaSource> source;
    TSessions::iterator it;
    {
        AutoThreadLocker locker(s_locker);
        it = s_sessions.find(session ? session : "");
        if(it == s_sessions.end())
        {
            return rtsp_server_reply_pause(rtsp, 454);
        }

        source = it->second.media;
        it->second.status = 2;
    }

    source->Pause();
    return rtsp_server_reply_pause(rtsp, 200);
}

static int rtsp_onteardown(void* /*ptr*/, rtsp_server_t* rtsp, const char* /*uri*/,
    const char* session)
{
    {
        AutoThreadLocker locker(s_locker);
        TSessions::iterator it = s_sessions.find(session ? session : "");
        if(it == s_sessions.end())
        {
            return rtsp_server_reply_teardown(rtsp, 454);
        }

        s_sessions.erase(it);
    }

    return rtsp_server_reply_teardown(rtsp, 200);
}

static int rtsp_onannounce(void* /*ptr*/, rtsp_server_t* rtsp,
    const char* uri, const char* sdp, int len)
{
    return rtsp_server_reply_announce(rtsp, 200);
}

static int rtsp_onrecord(void* /*ptr*/, rtsp_server_t* rtsp, const char* uri,
    const char* session, const int64_t *npt, const double *scale)
{
    return rtsp_server_reply_record(rtsp, 200, NULL, NULL);
}

static int rtsp_onoptions(void* ptr, rtsp_server_t* rtsp, const char* uri)
{
    return rtsp_server_reply_options(rtsp, 200);
}

static int rtsp_ongetparameter(void* ptr, rtsp_server_t* rtsp, const char* uri,
    const char* session, const void* content, int bytes)
{
    return rtsp_server_reply_get_parameter(rtsp, 200, NULL, 0);
}

static int rtsp_onsetparameter(void* ptr, rtsp_server_t* rtsp, const char* uri,
    const char* session, const void* content, int bytes)
{
    return rtsp_server_reply_set_parameter(rtsp, 200);
}

static int rtsp_onclose(void* /*ptr2*/)
{
    xlog_dbg("rtsp close");
    return 0;
}

static void rtsp_onerror(void* /*param*/, rtsp_server_t* rtsp, int code)
{
    xlog_war("rtsp_onerror code=%d, rtsp=%p\n", code, (void*)rtsp);

    AutoThreadLocker locker(s_locker);
    for (auto it = s_sessions.begin(); it != s_sessions.end(); ++it)
    {
        if (rtsp == it->second.rtsp)
        {
            it->second.media->Pause();
            s_sessions.erase(it);
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    xlog_dbg("hello");

    void *tcp = nullptr;

    do {
        aio_worker_init(N_AIO_THREAD);

        struct aio_rtsp_handler_t handler{};
        handler.base.ondescribe = rtsp_ondescribe;
        handler.base.onsetup = rtsp_onsetup;
        handler.base.onplay = rtsp_onplay;
        handler.base.onpause = rtsp_onpause;
        handler.base.onteardown = rtsp_onteardown;
        handler.base.close = rtsp_onclose;
        handler.base.onannounce = rtsp_onannounce;
        handler.base.onrecord = rtsp_onrecord;
        handler.base.onoptions = rtsp_onoptions;
        handler.base.ongetparameter = rtsp_ongetparameter;
        handler.base.onsetparameter = rtsp_onsetparameter;
        handler.onerror = rtsp_onerror;

        tcp = rtsp_server_listen("0.0.0.0", 8554, &handler, nullptr);
        if (nullptr == tcp) {
            xlog_err("listen failed");
            break;
        }
        xlog_dbg("listen successful, press Enter to start serving");

        getchar();
        xlog_dbg("serving...");

        while (1)
        {
            system_sleep(5);

            AutoThreadLocker locker(s_locker);
            for (auto &[_, session] : s_sessions)
            {
                if (1 == session.status)
                    session.media->Play();
            }
        }
    } while (false);

    aio_worker_clean(N_AIO_THREAD);
    if (nullptr != tcp) {
        rtsp_server_unlisten(tcp);
        tcp = nullptr;
    }

    return 0;
}
