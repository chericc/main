#include <libwebsockets.h>

#include <string.h>

static int callback_example(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED: {
            lwsl_user("Connected to server\n");
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            lwsl_user("Received data: %s\n", (char *)in);
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            lwsl_user("Sending message\n");
            const char *msg = "Hello, WebSocket!";
            size_t msg_len = strlen(msg);
            unsigned char buf[LWS_PRE + msg_len];
            memcpy(&buf[LWS_PRE], msg, msg_len);
            lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
            break;
        }
        default: {
            break;
        }
    }
    return 0;
}

int main(void) {
    struct lws_context_creation_info info;

    struct lws_protocols protocols[2] = {
        { 
                .name = "example-protocol", 
                .callback = callback_example, 
                .per_session_data_size = 0, 
                .rx_buffer_size = 4096 
            },
    };

    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    struct lws_context *context = lws_create_context(&info);
    struct lws_client_connect_info ccinfo = { 0 };
    ccinfo.context = context;
    ccinfo.address = "example.com";
    ccinfo.port = 80;
    ccinfo.path = "/";
    ccinfo.protocol = "example-protocol";
    lws_client_connect_via_info(&ccinfo);

    while (lws_service(context, 1000) >= 0) {}
    lws_context_destroy(context);
    return 0;
}
