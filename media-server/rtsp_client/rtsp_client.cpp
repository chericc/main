#include "rtsp_client.h"

#include <pthread.h>
#include <sys/prctl.h>

#include "rtsp-client.h"
#include "sockutil.h"
#include "sdp.h"

#include "xlog.hpp"

namespace {

#define GENERAL_NET_TIMEOUT_MS  2000

typedef enum {
    RunningStatus_None,
    RunningStatus_Ok,
    RunningStatus_Failed,
    RunningStatus_Butt,
} RunningStatus;

struct rtsp_client_param_inner_t {
    char host[64];
    char file[64];
    char username[64];
    char password[64];
    char port[64];
};

struct rtsp_client_obj_t {
    rtsp_client_param_inner_t param;

    rtsp_client_t *rtsp;
    socket_t socket;

    int trd_recv_flag;
    int trd_recv_running_flag;
    pthread_t tid_recv_trd;
    RunningStatus status;
};

int rtp_receive_tcp_payload(rtsp_client_obj_t *obj, uint8_t interleave1, uint8_t interleave2, int payload, const char* encoding)
{
    xlog_war("rtp payload: interleave1,interleave2,payload,encoding=%d,%d,%d,%s\n", 
        interleave1, interleave2, payload, encoding);
    return 0;
}

int rtp_receive_tcp_input(rtsp_client_obj_t *obj, uint8_t channel, const void* data, uint16_t bytes)
{
    xlog_war("rtp input: ch,bytes=%d,%d\n", channel, bytes);
    return 0;
}

int cb_send(void* param, const char* uri, const void* req, size_t bytes)
{
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    xlog_war("send: %s\n", (char*)req);
    int ret = socket_send_all_by_time(obj->socket, req, bytes, 0, GENERAL_NET_TIMEOUT_MS);
    return ret;
}

int cb_rtpport(void* param, int media, const char* source, unsigned short rtp[2], char* ip, int len)
{
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    xlog_war("on rtp port\n");
    do {

        int media_type = rtsp_client_get_media_type(obj->rtsp, media);
        if (media_type != SDP_M_MEDIA_AUDIO && media_type != SDP_M_MEDIA_VIDEO) {
            xlog_err("not audio or video(%d)\n", media_type);
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
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    xlog_war("cb_ondescribe: %s\n", sdp);
    int ret = rtsp_client_setup(obj->rtsp, sdp, len);

    return ret;
}

int cb_onsetup(void* param, int timeout, int64_t duration)
{
    int error_flag = 0;
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    xlog_war("on setup\n");

    do {
        uint64_t npt = 0;
        int ret = rtsp_client_play(obj->rtsp, &npt, nullptr);
        if (ret != 0) {
            xlog_err("rtsp_client_play failed\n");
            break;
        }

        int media_count = rtsp_client_media_count(obj->rtsp);
        xlog_war("client media count: %d\n", media_count);
        for (int i = 0; i < media_count; ++i) {
            const struct rtsp_header_transport_t *transport = nullptr;
            const char *encoding = nullptr;
            int payload = 0;

            transport = rtsp_client_get_media_transport(obj->rtsp, i);
            encoding = rtsp_client_get_media_encoding(obj->rtsp, i);
            payload = rtsp_client_get_media_payload(obj->rtsp, i);

            xlog_war("transport[%d]=%d(1 udp,2 tcp,3 raw)\n", i, transport->transport);

            if (RTSP_TRANSPORT_RTP_UDP == transport->transport) {
                xlog_err("udp not support now\n");
            } else {
                rtp_receive_tcp_payload(obj, transport->interleaved1, transport->interleaved2, 
                    payload, encoding);
            }
        }
    } while (0);

    return 0;
}

int cb_onteardown(void* param)
{
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    xlog_war("cb_onteardown\n");
    return 0;
}

int cb_onplay(void* param, int media, const uint64_t *nptbegin, const uint64_t *nptend, const double *scale, const struct rtsp_rtp_info_t* rtpinfo, int count)
{
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    xlog_war("cb_onplay\n");
    return 0;
}

int cb_onpause(void* param)
{
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    xlog_war("cb_onpause\n");
    return 0;
}

void cb_onrtp(void* param, uint8_t channel, const void* data, uint16_t bytes)
{
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)param;
    // xlog_war("on rtp: [ch,bytes]=[%d,%d]\n", channel, bytes);
    rtp_receive_tcp_input(obj, channel, data, bytes);
    return ;
}

void *trd_recv_rtsp(void *args)
{
    rtsp_client_obj_t *obj = (rtsp_client_obj_t *)args;

    prctl(PR_SET_NAME, "rtsp_client_recv");

    socket_setnonblock(obj->socket, 0);

    uint8_t buf[64 * 1024];
    while (obj->trd_recv_running_flag) {
        int ret_recv = socket_recv(obj->socket, buf, sizeof(buf), 0);
        if (ret_recv <= 0) {
            xlog_err("recv end or error\n");
            break;
        }
        int ret_input = rtsp_client_input(obj->rtsp, buf, ret_recv);
        if (ret_input != 0) {
            xlog_err("input failed\n");
        }
    }

    return nullptr;
}

int rtsp_client_deinit(rtsp_client_obj_t *obj)
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
    } while (false);

    return 0;
}

int rtsp_client_init(rtsp_client_obj_t *obj)
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

        xlog_war("url: %s\n", url);

        struct rtsp_client_handler_t handler = {};
        handler.send = cb_send;
        handler.rtpport = cb_rtpport;
        handler.ondescribe = cb_ondescribe;
        handler.onsetup = cb_onsetup;
        handler.onplay = cb_onplay;
        handler.onpause = cb_onpause;
        handler.onteardown = cb_onteardown;
        handler.onrtp = cb_onrtp;
        xlog_war("url:%s, username:%s, password:%s\n", url, obj->param.username, obj->param.password);
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

rtsp_client_handle_t rtsp_client_start(const rtsp_client_param *param)
{
    int error_flag = 0;
    rtsp_client_obj_t *client = nullptr;

    do {
        client = (rtsp_client_obj_t*)malloc(sizeof(rtsp_client_obj_t));
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

    return reinterpret_cast<rtsp_client_handle_t>(client);
}

int rtsp_client_stop(rtsp_client_handle_t handle)
{
    rtsp_client_obj_t *client = reinterpret_cast<rtsp_client_obj_t*>(handle);
    do {
        if (!client) {
            xlog_err("client is null\n");
            break;
        }

        rtsp_client_deinit(client);
    } while (0);

    return 0;
}

