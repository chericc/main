
#include <stdarg.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/http.h>

#include "xlog.hpp"

struct options
{
    int port = 0;
    const char *docroot = nullptr;
};

static void print_usage(FILE *out, int argc, char **argv)
{
    fprintf(out, 
        "Usage: %s [ OPTS ] <docroot>\n"
        " -h    - print help\n"
        " -p    - port\n", argv[0]);
    return ;
}

static struct options parse_opts(int argc, char **argv)
{
    struct options sopt = {};
    int opt = 0;

    while ((opt = getopt(argc, argv, "hp:")) != -1)
    {
        switch (opt)
        {
            case 'h': print_usage(stdout, argc, argv); exit(0); break;
            case 'p': sopt.port = atoi(optarg); break;
            default: xlog_err("Unknown option %c", opt); break;
        }
    }

    if (optind >= argc || (argc - optind) > 1)
    {
        print_usage(stdout, argc, argv);
        exit(0);
    }
    sopt.docroot = argv[optind];
    return sopt;
}

static bool signal_ignore()
{
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        xlog_err("signal failed");
        return false;
    }
    return true;
}

static void on_term(int sig, short events, void *arg)
{
    auto base = reinterpret_cast<event_base*>(arg);
    event_base_loopbreak(base);
    xlog_dbg("Interrupted, Terminating");
    return ;
}

static void dump_request_cb(struct evhttp_request *req, void *arg)
{
    switch (evhttp_request_get_command(req))
    {
        
    }
}

static void send_document_cb(struct evhttp_request *req, void *arg)
{
    xlog_dbg("in");
    xlog_dbg("out");
    return ;
}

int main(int argc, char **argv)
{
    xlog_dbg("in");

    struct event_base *base = nullptr;
    struct evhttp *http = nullptr;
    struct evhttp_bound_socket *handle_v4 = nullptr;
    struct evhttp_bound_socket *handle_v6 = nullptr;
    struct event *term = NULL;
    struct options sopt = parse_opts(argc, argv);

    do 
    {
        if (!signal_ignore())
        {
            break;
        }

        base = event_base_new();
        if (!base) 
        {
            xlog_err("event_base_new failed");
            break;
        }

        http = evhttp_new(base);
        if (!http)
        {
            xlog_err("evhttp_new failed");
            break;
        }

        evhttp_set_gencb(http, send_document_cb, &sopt);

        handle_v4 = evhttp_bind_socket_with_handle(http, "0.0.0.0", sopt.port);
        handle_v6 = evhttp_bind_socket_with_handle(http, "::/0", sopt.port);

        if (!handle_v4 && !handle_v6)
        {
            xlog_err("bind failed");
            break;
        }

        term = evsignal_new(base, SIGINT, on_term, base);
        if (!term)
        {
            xlog_err("evsignal_new failed");
            break;
        }
        if (event_add(term, nullptr))
        {
            xlog_err("event_add failed");
            break;
        }

        event_base_dispatch(base);
    }
    while (false);

    xlog_dbg("out");
    return 0;
}

