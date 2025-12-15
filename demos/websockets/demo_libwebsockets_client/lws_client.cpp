#include <libwebsockets.h>

#include <string.h>
#include <stdio.h>
#include <array>

#include "xlog.h"

static int callback_example(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED: {
            xlog_dbg("Connected to server\n");
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            xlog_dbg("Received data: {}\n", (char *)in);
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            xlog_dbg("Sending message\n");
            const char *msg = "Hello, WebSocket!";
            size_t msg_len = strlen(msg);
            unsigned char buf[LWS_PRE + 128] = {};
            memcpy(&buf[LWS_PRE], msg, msg_len + 1);
            lws_write(wsi, &buf[LWS_PRE], msg_len + 1, LWS_WRITE_TEXT);
            break;
        }
        default: {
            break;
        }
    }
    return 0;
}

static void my_lws_log_emit_t(int level, const char *line)
{
    xlog_dbg("lws log: {}", line);
}

int main(void) {
    struct lws_context_creation_info info;

    struct lws_protocols protocols[2] = {
        { 
                "my-test-lws-client", 
                callback_example, 
                0,
                4096,
                0, nullptr, 0, 
            },
            LWS_PROTOCOL_LIST_TERM,
    };

    lws_set_log_level(LLL_ERR | LLL_WARN, my_lws_log_emit_t);

    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    struct lws_context *context = lws_create_context(&info);
    struct lws_client_connect_info ccinfo = {};
    ccinfo.context = context;
    ccinfo.address = "127.0.0.1";
    ccinfo.port = 9008;
    ccinfo.path = "/app";
    ccinfo.protocol = "my-test-lws-server";
    lws_client_connect_via_info(&ccinfo);

    while (lws_service(context, 1000) >= 0) {}
    lws_context_destroy(context);
    return 0;
}
