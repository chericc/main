#include <event2/dns.h>
#include <event2/event.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <array>
#include <cstdint>
#include <string>

#include "event2/util.h"
#include "xlog.hpp"

static void event_log(int is_warning, const char* msg) {
    if (is_warning) {
        xlog_war("libevent: %s", msg);
    } else {
        // xlog_dbg("libevent: %s", msg);
    }
}

void getaddrinfo_cb(int result, struct evutil_addrinfo* res, void* arg) {
    const char* name = (const char*)arg;
    int i;
    struct evutil_addrinfo* first_ai = res;

    if (result) {
        xlog_dbg("%s: %s", name, evutil_gai_strerror(result));
    }

    if (res && res->ai_canonname) {
        xlog_dbg("    %s ==> %s", name, res->ai_canonname);
    }

    for (i = 0; res; res = res->ai_next, ++i) {
        std::array<char, 128> buf;
        if (res->ai_family == PF_INET) {
            auto* sin = (struct sockaddr_in*)res->ai_addr;
            evutil_inet_ntop(AF_INET, &sin->sin_addr, buf.data(), buf.size());
            xlog_dbg("IPv4 [%d] %s: %s", i, name, buf.data());
        } else if (res->ai_family == PF_INET6) {
            auto* sin6 = (struct sockaddr_in6*)res->ai_addr;
            evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf.data(),
                             buf.size());
            xlog_dbg("IPv6 [%d] %s: %s", i, name, buf.data());
        } else {
            xlog_dbg("family not support");
        }
    }

    if (first_ai) evutil_freeaddrinfo(first_ai);
}

int main() {
    xlog_setoutput({stdout});
    xlog_setmask(XLOG_ALLOW_ALL);

    struct event_base* event_base = nullptr;
    struct evdns_base* evdns_base = nullptr;

    event_base = event_base_new();
    evdns_base =
        evdns_base_new(event_base, EVDNS_BASE_INITIALIZE_NAMESERVERS |
                                       EVDNS_BASE_DISABLE_WHEN_INACTIVE);

    evdns_set_log_fn(event_log);

    //
    std::string url = "baidu.com";
    {
        struct evutil_addrinfo hints = {};
        hints.ai_family = PF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = EVUTIL_AI_CANONNAME;
        evdns_getaddrinfo(evdns_base, url.c_str(), nullptr, &hints,
                          getaddrinfo_cb, nullptr);
    }

    event_base_dispatch(event_base);
    evdns_base_free(evdns_base, 1);
    event_base_free(event_base);

    return 0;
}