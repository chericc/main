#include "rtsp_client_wrapper.h"

#include <pthread.h>
#include <array>
#include <sys/prctl.h>

#include "rtsp-client.h"
#include "sockutil.h"
#include "sdp.h"
#include "rtp-profile.h"
#include "rtp-demuxer.h"
#include "xlog.h"
#include "packet_stable.hpp"

#define GENERAL_NET_TIMEOUT_MS  2000

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) ((sizeof(arr))/(sizeof(arr[0])))
#endif 

namespace {

typedef enum {
    RunningStatus_None,
    RunningStatus_Ok,
    RunningStatus_Failed,
    RunningStatus_Butt,
} RunningStatus;

struct rtsp_client_wrapper_param_inner_t {
    char host[64];
    char file[64];
    char username[64];
    char password[64];
    char port[64];

    rtsp_client_wrapper_data_cb data_cb;
};

struct rtsp_client_wrapper_obj_t;

struct rdp_demuxer_ctx_t {
    int valid;
    int idx;
    rtsp_client_wrapper_obj_t *obj;
    struct rtp_demuxer_t *demuxer;
    char encoding[64];
    int payload;
};

struct rtsp_client_wrapper_obj_t {
    rtsp_client_wrapper_param_inner_t param;

    rtsp_client_t *rtsp;
    socket_t socket;

    int trd_recv_flag;
    int trd_recv_running_flag;
    pthread_t tid_recv_trd;
    RunningStatus status;

    rdp_demuxer_ctx_t demuxer[8];
    PacketStable *stable[8];
};

void on_packet_stable_data_cb(const void *data, size_t bytes, void *user_data)
{
    rdp_demuxer_ctx_t *demux_ctx = reinterpret_cast<rdp_demuxer_ctx_t*>(user_data);

    do {
        auto data_cb = demux_ctx->obj->param.data_cb;
        if (!data_cb) {
            xlog_err("null data cb\n");
            break;
        }
        
        data_cb(demux_ctx->idx, demux_ctx->payload, data, bytes);
    } while (false);

    return ;
}

int rtp_onpacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags)
{
    rdp_demuxer_ctx_t *demux_ctx = reinterpret_cast<rdp_demuxer_ctx_t*>(param);

    // if (demux_ctx->idx == 0) {
    //     xlog_dbg("idx:{}, encoding: {}, ts: %d, bytes: %d, flags: %d\n", 
    //         demux_ctx->idx, demux_ctx->encoding, timestamp / 90, bytes, flags);
    // }
    do {

        int idx = demux_ctx->idx;
        if (idx < 0 || idx >= (int)ARRAY_SIZE(demux_ctx->obj->stable)) {
            xlog_err("invalid idx({})\n", idx);
            break;
        }

        if (!demux_ctx->obj->stable[idx]) {
            demux_ctx->obj->stable[idx] = new PacketStable(on_packet_stable_data_cb, demux_ctx);
        }

        if (demux_ctx->payload == RTP_PAYLOAD_H265
            || demux_ctx->payload == RTP_PAYLOAD_H264) {
            
            static std::mutex mutex;
            static std::vector<uint8_t> buffer;
            std::lock_guard<std::mutex> lock(mutex);

            const uint8_t start_code[] = { 0, 0, 0, 1 };
            size_t combined_size = sizeof(start_code) + bytes;

            if (buffer.size() < combined_size) {
                buffer.resize(combined_size);
                xlog_dbg("buffer size:{}\n", (int)combined_size);
            }

            memcpy(buffer.data(), start_code, sizeof(start_code));
            memcpy(buffer.data() + sizeof(start_code), packet, bytes);

            demux_ctx->obj->stable[idx]->push(buffer.data(), combined_size, timestamp / 90);
        }

    } while (false);

    return 0;
}

int rtp_receive_tcp_setup(rtsp_client_wrapper_obj_t *obj, uint8_t interleave1, uint8_t interleave2, int payload, const char* encoding)
{
    xlog_war("rtp payload: interleave1,interleave2,payload,encoding={},{},{},{}\n", 
        interleave1, interleave2, payload, encoding);

    int error_flag = 0;

    do {
        int demuxer_max_size = ARRAY_SIZE(obj->demuxer);
        int chn_idx = interleave1 / 2;
        if (chn_idx < 0 || chn_idx >= demuxer_max_size) {
            xlog_war("chn idx not support({})\n", chn_idx);
            error_flag = 1;
            break;
        }

        rdp_demuxer_ctx_t *demux_ctx = &obj->demuxer[chn_idx];
        if (demux_ctx->valid) {
            xlog_err("demux is used\n");
            error_flag = 1;
            break;
        }
        demux_ctx->valid = 1;

        snprintf(demux_ctx->encoding, sizeof(demux_ctx->encoding), "%s", encoding);
        demux_ctx->idx = chn_idx;
        demux_ctx->obj = obj;
        demux_ctx->payload = payload;

        const struct rtp_profile_t *profile = nullptr;
        profile = rtp_profile_find(payload);

        xlog_war("demux create: idx={}\n", chn_idx);

        const int jitter_ms = 100;
        const int frequency_dft = 90000;
        demux_ctx->demuxer = rtp_demuxer_create(jitter_ms, 
            profile ? profile->frequency : frequency_dft,
            payload, encoding, rtp_onpacket, demux_ctx);
        
    } while (false);

    return error_flag ? -1 : 0;
}

int rtp_receive_tcp_input(rtsp_client_wrapper_obj_t *obj, uint8_t channel, const void* data, uint16_t bytes)
{
    // xlog_war("rtp input: ch,bytes={},{}\n", channel, bytes);
    
    int error_flag = 0;

    do {
        int chn_idx = channel / 2;
        int demuxer_max_size = ARRAY_SIZE(obj->demuxer);
        if (chn_idx < 0 || chn_idx >= demuxer_max_size) {
            xlog_err("invalid channel: {}\n", channel);
            error_flag = 1;
            break;
        }

        rdp_demuxer_ctx_t *demuxer_ctx = &obj->demuxer[chn_idx];
        if (!demuxer_ctx->valid) {
            xlog_err("demuxer invalid(chn={})\n", chn_idx);
            error_flag = 1;
            break;
        }

        int ret_input = rtp_demuxer_input(demuxer_ctx->demuxer, data, bytes);
        if (ret_input < 0) {
            xlog_err("input failed\n");
            error_flag = 1;
            break;
        }
    } while (false);

    return error_flag ? -1 : 0;
}

int cb_send(void* param, const char* uri, const void* req, size_t bytes)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    xlog_war("send: {}\n", (char*)req);
    int ret = socket_send_all_by_time(obj->socket, req, bytes, 0, GENERAL_NET_TIMEOUT_MS);
    return ret;
}

int cb_rtpport(void* param, int media, const char* source, unsigned short rtp[2], char* ip, int len)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    xlog_war("on rtp port\n");
    do {

        int media_type = rtsp_client_get_media_type(obj->rtsp, media);
        if (media_type != SDP_M_MEDIA_AUDIO && media_type != SDP_M_MEDIA_VIDEO) {
            xlog_err("not audio or video({})\n", media_type);
            break;
        }

        // tcp only
        rtp[0] = 2 * media;
        rtp[1] = 2 * media + 1;
        
    } while (0);

    return RTSP_TRANSPORT_RTP_TCP;
}

int cb_ondescribe(void* param, const char* sdp, int len)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    xlog_war("cb_ondescribe: {}\n", sdp);
    int ret = rtsp_client_setup(obj->rtsp, sdp, len);

    return ret;
}

int cb_onsetup(void* param, int timeout, int64_t duration)
{
    int error_flag = 0;
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    xlog_war("on setup\n");

    do {
        uint64_t npt = 0;
        int ret = rtsp_client_play(obj->rtsp, &npt, nullptr);
        if (ret != 0) {
            xlog_err("rtsp_client_play failed\n");
            break;
        }

        int media_count = rtsp_client_media_count(obj->rtsp);
        xlog_war("client media count: {}\n", media_count);
        for (int i = 0; i < media_count; ++i) {
            const struct rtsp_header_transport_t *transport = nullptr;
            const char *encoding = nullptr;
            int payload = 0;

            transport = rtsp_client_get_media_transport(obj->rtsp, i);
            encoding = rtsp_client_get_media_encoding(obj->rtsp, i);
            payload = rtsp_client_get_media_payload(obj->rtsp, i);

            xlog_war("transport[{}]={}(1 udp,2 tcp,3 raw)\n", i, transport->transport);

            if (RTSP_TRANSPORT_RTP_UDP == transport->transport) {
                xlog_err("udp not support now\n");
            } else {
                rtp_receive_tcp_setup(obj, transport->interleaved1, transport->interleaved2, 
                    payload, encoding);
            }
        }
    } while (0);

    return 0;
}

int cb_onteardown(void* param)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    xlog_war("cb_onteardown\n");
    return 0;
}

int cb_onplay(void* param, int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale, const struct rtsp_rtp_info_t* rtpinfo, int count)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    xlog_war("cb_onplay\n");
    return 0;
}

int cb_onpause(void* param)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    xlog_war("cb_onpause\n");
    return 0;
}

void cb_onrtp(void* param, uint8_t channel, const void* data, uint16_t bytes)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)param;
    // xlog_war("on rtp: [ch,bytes]=[{},{}]\n", channel, bytes);
    rtp_receive_tcp_input(obj, channel, data, bytes);
    return ;
}

void *trd_recv_rtsp(void *args)
{
    rtsp_client_wrapper_obj_t *obj = (rtsp_client_wrapper_obj_t *)args;

    prctl(PR_SET_NAME, "rtsp_client_recv");

    socket_setnonblock(obj->socket, 0);
    socket_setrecvtimeout(obj->socket, 1);
    socket_setkeepalive(obj->socket, true);

    uint8_t buf[64 * 1024];
    while (obj->trd_recv_running_flag) {
        errno = 0;
        int ret_recv = socket_recv(obj->socket, buf, sizeof(buf), 0);
        int errno_cache = errno;
        if (ret_recv == 0) {
            xlog_war("recv eof\n");
            break;
        } else if (ret_recv < 0) {
            if (errno_cache == EAGAIN || errno_cache == EWOULDBLOCK) {
                xlog_war("recv timeout(error:{})\n", strerror(errno_cache));
                continue;
            } else {
                xlog_err("recv error(error:{})\n", strerror(errno_cache));
            break;
            }
        }
        int ret_input = rtsp_client_input(obj->rtsp, buf, ret_recv);
        if (ret_input != 0) {
            xlog_err("input failed\n");
        }
    }

    return nullptr;
}

int rtsp_client_deinit(rtsp_client_wrapper_obj_t *obj)
{
    do {
        if (!obj) {
            xlog_err("obj is null\n");
            break;
        }

        if (obj->trd_recv_flag) {
            xlog_war("join thread\n");
            obj->trd_recv_running_flag = 0;
            pthread_join(obj->tid_recv_trd, nullptr);
            obj->trd_recv_flag = 0;
        }

        if (obj->rtsp) {
            rtsp_client_teardown(obj->rtsp);
            rtsp_client_destroy(obj->rtsp);
        } else {
            xlog_err("rtsp is not initialized\n");
        }

        if (obj->socket != socket_invalid) {
            socket_close(obj->socket);
        } else {
            xlog_err("socket is not created\n");
        }
        
        for (auto &ref : obj->demuxer) {
            if (ref.demuxer) {
                xlog_dbg("destroy demuxer\n");
                rtp_demuxer_destroy(&ref.demuxer);
                ref.demuxer = nullptr;
            }
        }
        for (auto &ref : obj->stable) {
            if (ref) {
                xlog_dbg("destroy stable\n");
                delete ref;
                ref = nullptr;
            }
        }
    } while (false);

    return 0;
}

int rtsp_client_init(rtsp_client_wrapper_obj_t *obj)
{
    int error_flag = 0;

    do {
        int ret = 0;

        // socket_init();
        obj->socket = socket_connect_host(obj->param.host, atoi(obj->param.port), GENERAL_NET_TIMEOUT_MS);
        if (obj->socket == socket_invalid) {
            xlog_err("connect failed\n");
            error_flag = 1;
            break;
        }

        char url[256] = {};
        snprintf(url, sizeof(url), "rtsp://%s/%s", obj->param.host, obj->param.file);

        xlog_war("url: {}\n", url);

        struct rtsp_client_handler_t handler = {};
        handler.send = cb_send;
        handler.rtpport = cb_rtpport;
        handler.ondescribe = cb_ondescribe;
        handler.onsetup = cb_onsetup;
        handler.onplay = cb_onplay;
        handler.onpause = cb_onpause;
        handler.onteardown = cb_onteardown;
        handler.onrtp = cb_onrtp;
        xlog_war("url:{}, username:{}, password:{}\n", url, obj->param.username, obj->param.password);
        obj->rtsp = rtsp_client_create(url, obj->param.username, obj->param.password, &handler, obj);
        if (obj->rtsp == nullptr) {
            xlog_err("rtsp create failed\n");
            error_flag = 1;
            break;
        }

        ret = rtsp_client_describe(obj->rtsp);
        if (ret != 0) {
            xlog_err("rtsp_client_describe failed\n");
            error_flag = 1;
            break;
        }

        obj->trd_recv_running_flag = 1;
        ret = pthread_create(&obj->tid_recv_trd, NULL, trd_recv_rtsp, obj);
        if (ret != 0) {
            xlog_err("pthread_create failed\n");
            error_flag = 1;
            break;
        }
        obj->trd_recv_flag = 1;
    } while (false);

    if (error_flag) {
        rtsp_client_deinit(obj);
    }

    return error_flag ? -1 : 0;
}

}

rtsp_client_wrapper_handle_t rtsp_client_wrapper_start(const rtsp_client_wrapper_param *param)
{
    int error_flag = 0;
    rtsp_client_wrapper_obj_t *client = nullptr;

    do {
        client = (rtsp_client_wrapper_obj_t*)malloc(sizeof(rtsp_client_wrapper_obj_t));
        if (!client) {
            xlog_err("malloc failed\n");
            error_flag = 1;
            break;
        }
        memset(client, 0, sizeof(*client));
        client->socket = socket_invalid;
        client->status = RunningStatus_None;

        snprintf(client->param.host, sizeof(client->param.host), "%s", param->host);
        snprintf(client->param.file, sizeof(client->param.file), "%s", param->file);
        snprintf(client->param.username, sizeof(client->param.username), "%s", param->username);
        snprintf(client->param.password, sizeof(client->param.password), "%s", param->password);
        snprintf(client->param.port, sizeof(client->param.port), "%s", param->port);
        client->param.data_cb = param->data_cb;

        int ret = 0;
        ret = rtsp_client_init(client);
        if (ret < 0) {
            xlog_err("init failed\n");
            error_flag = 1;
            break;
        }
    } while (0);

    if (error_flag) {
        if (client) {
            free(client);
            client = nullptr;
        }
    }

    rtsp_client_wrapper_handle_t handle = rtsp_client_wrapper_handle_invalid;
    if (client) {
        handle = reinterpret_cast<rtsp_client_wrapper_handle_t>(client);
    }
    return handle;
}

int rtsp_client_wrapper_destroy(rtsp_client_wrapper_handle_t handle)
{
    rtsp_client_wrapper_obj_t *client = reinterpret_cast<rtsp_client_wrapper_obj_t*>(handle);
    do {
        if (!client) {
            xlog_err("client is null\n");
            break;
        }

        rtsp_client_deinit(client);
        free(client);
        client = nullptr;
    } while (0);

    return 0;
}

