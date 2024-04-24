#include <array>

#include <stdarg.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>

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
    xlog_dbg("interrupted, terminating");
    auto base = reinterpret_cast<event_base*>(arg);
    event_base_loopbreak(base);
    return ;
}

static const char *http_req_type_str(enum evhttp_cmd_type type)
{
    const char *typestr = "";
    switch (type)
    {
        case EVHTTP_REQ_GET: typestr = "GET"; break;
        case EVHTTP_REQ_POST: typestr = "POST"; break;
        case EVHTTP_REQ_HEAD: typestr = "HEAD"; break;
        case EVHTTP_REQ_PUT: typestr = "PUT"; break;
        case EVHTTP_REQ_DELETE: typestr = "DELETE"; break;
        case EVHTTP_REQ_OPTIONS: typestr = "OPTIONS"; break;
        case EVHTTP_REQ_TRACE: typestr = "TRACE"; break;
        case EVHTTP_REQ_CONNECT: typestr = "CONNECT"; break;
        case EVHTTP_REQ_PATCH: typestr = "PATCH"; break;
        default: typestr = "unknown"; break;
    }
    return typestr;
}

static std::string dump_req_content(struct evhttp_request *req)
{
    FILE *fp = nullptr;
    char *streambuf = nullptr;
    size_t streambufsize = 0;
    std::string str;

    do 
    {
        if (!req)
        {
            xlog_err("req is null");
            break;
        }

        fp = open_memstream(&streambuf, &streambufsize);
        if (!fp)
        {
            xlog_err("open_memstream failed");
            break;
        }

        fprintf(fp, "Header: \n");
        auto *header_s = evhttp_request_get_input_headers(req);
        if (header_s)
        {
            for (struct evkeyval *header = header_s->tqh_first; 
                    header; header = header->next.tqe_next)
            {
                fprintf(fp, " %s: %s\n", header->key, header->value);
            }
        }

        fprintf(fp, "Data: \n");
        auto input = evhttp_request_get_input_buffer(req);
        while (input && evbuffer_get_length(input))
        {
            std::array<uint8_t, 64> buf;
            int n = evbuffer_remove(input, buf.data(), buf.size());
            if (n > 0)
            {
                fwrite(buf.data(), 1, n, fp);
            }
        }
    }
    while (0);

    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }

    if (streambuf)
    {
        if (streambufsize > 0)
        {
            str.assign(streambuf);
        }
        
        free(streambuf);
        streambuf = nullptr;
    }

    return str;
}

static void dump_request_cb(struct evhttp_request *req, void *arg)
{
    auto req_type = evhttp_request_get_command(req);
    const char *req_type_str = http_req_type_str(req_type);

    auto content = dump_req_content(req);

    xlog_dbg("received a %s request for %s\n%s", 
        req_type_str, evhttp_request_get_uri(req), content.c_str());
    
    evhttp_send_reply(req, 200, "OK", nullptr);
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

        evhttp_set_gencb(http, dump_request_cb, &sopt);

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

