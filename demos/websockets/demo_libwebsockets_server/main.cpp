

#include <unistd.h>
#include "lws_server_wrapper.h"
#include "xlog.hpp"

int main()
{
    LwsServerWrapperParam param = {};
    lws_server_wrapper_init(&param);

    for (int i = 0; i < 10; ++i) {
        xlog_dbg("wait\n");
        sleep(5);
    }

    lws_server_wrapper_deinit();

    return 0;
}