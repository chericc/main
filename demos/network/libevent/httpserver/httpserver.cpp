#include <array>
#include <cstring>
#include <stdarg.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>

#include "xlog.hpp"
#include "xos_independent.hpp"

struct options
{
    int port = 0;
    const char *docroot = nullptr;
};

static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postscript" }
};

static void print_usage(FILE *out, int , char **argv)
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
    xlog_dbg("interrupted(sig=%d, events=%#hx), terminating", sig, events);
    auto base = reinterpret_cast<event_base*>(arg);
    event_base_loopbreak(base);
    return ;
}

static const char *guess_content_type(const char *path)
{
    bool found = false;
    const char *content_type = nullptr;

    do 
    {
        const char *last = nullptr;
        const char *extension = nullptr;
        last = strrchr(path, '.');

        if (!last)
        {
            break;
        }

        extension = last + 1;
        for (auto const &ref : content_type_table)
        {
            if (!strcasecmp(ref.extension, extension))
            {
                content_type = ref.content_type;
                found = true;
                break;
            }
        }
    }
    while (false);
    
    if (!found)
    {
        content_type = "application/octet-stream";
    }
    return content_type;
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

static void dump_request_cb(struct evhttp_request *req, void *)
{
    auto req_type = evhttp_request_get_command(req);
    const char *req_type_str = http_req_type_str(req_type);

    auto content = dump_req_content(req);

    xlog_dbg("received a %s request for %s\n%s", 
        req_type_str, evhttp_request_get_uri(req), content.c_str());
    
    evhttp_send_reply(req, 200, "OK", nullptr);
}

static int generate_html_of_path(evbuffer *evbuf, const char *root_path, const char *relative_path, int *error_code)
{
    bool error = true;
    DIR *dir = nullptr;
    int fd = -1;

    do
    {
        if (!evbuf || !root_path || !relative_path || !error_code)
        {
            xlog_err("null");
            break;
        }

        xlog_dbg("path:<%s, %s>", root_path, relative_path);

        std::string path_real = std::string() + root_path + "/" + relative_path;

        struct stat st = {};
        if (stat(path_real.c_str(), &st) < 0)
        {
            int errno_local = errno;
            std::array<char, 64> buf_str = {};
            x_strerror(errno_local, buf_str.data(), buf_str.size());
            xlog_err("stat failed, path(%s): %s", path_real.c_str(), buf_str.data());
            *error_code = HTTP_NOTFOUND;
            break;
        }

        if (S_ISDIR(st.st_mode))
        {
            dir = opendir(path_real.c_str());
            if (!dir)
            {
                xlog_err("opendir failed");
                *error_code = HTTP_NOTFOUND;
                break;
            }

            // std::string relative_path_with_trailing_slash = std::string() + relative_path + "/";
            const char *relative_path_with_trailing_slash = "";
            if (!strlen(relative_path) || relative_path[strlen(relative_path) - 1] != '/')
            {
                relative_path_with_trailing_slash = "/";
            }

            evbuffer_add_printf(evbuf,
R"(
<html>
 <head>
   <meta charset="UTF-8">
   <title>Index of %s</title>
   <base href="%s%s">
 </head>
 <body>
  <h1>Index of %s</h1>
  <ul>)"
            ,
            relative_path, 
            relative_path,
            relative_path_with_trailing_slash,
            relative_path);

            struct dirent *ent = nullptr;
            while ((ent = readdir(dir)))
            {
                const char *name = ent->d_name;
                std::string item_path = std::string() + name;
                evbuffer_add_printf(evbuf,
R"(
   <li><a href="%s">%s</a>
)"              , item_path.c_str(), name);
            }

            evbuffer_add_printf(evbuf, "</ul></body></html>\n");
        }
        else 
        {
            if ((fd = open(path_real.c_str(), O_RDONLY)) < 0)
            {
                xlog_err("open file failed");
                break;
            }

            struct stat st_local = {};
            if (fstat(fd, &st_local) < 0)
            {
                xlog_err("fstat failed");
                break;
            }

            evbuffer_add_file(evbuf, fd, 0, st_local.st_size);
        }

        error = false;
    }
    while (0);

    if (dir)
    {
        closedir(dir);
        dir = nullptr;
    }

    if (fd >= 0)
    {
        close(fd);
        fd = -1;
    }

    return error ? -1 : 0;
}

static void send_document_cb(struct evhttp_request *req, void *arg)
{
    xlog_dbg("in");

    struct evhttp_uri *parsesd_uri = nullptr;
    struct evbuffer *evbuf = nullptr;
    char *decoded_uri_path = nullptr;

    bool error = true;
    int http_error_code = 0;

    do 
    {
        auto *sopt = (struct options*)arg;

        if (!sopt)
        {
            xlog_err("null opt");
            break;
        }

        if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
        {
            xlog_err("request command is not GET, dump");
            dump_req_content(req);
            break;
        }

        const char *uri = evhttp_request_get_uri(req);
        if (!uri)
        {
            xlog_err("evhttp_request_get_uri failed");
            http_error_code = HTTP_NOTFOUND;
            break;
        }
        xlog_dbg("uri: %s", uri);

        decoded_uri_path = evhttp_uridecode(uri, 0, nullptr);
        if (!decoded_uri_path)
        {
            http_error_code = HTTP_NOTFOUND;
            xlog_err("evhttp_uridecode failed");
            break;
        }
        xlog_dbg("decoded_uri_path: %s", decoded_uri_path);

        parsesd_uri = evhttp_uri_parse(decoded_uri_path);
        if (!parsesd_uri)
        {
            xlog_err("evhttp_uri_parse failed");
            http_error_code = HTTP_BADREQUEST;
            break;
        }

        const char *uri_path = evhttp_uri_get_path(parsesd_uri);
        if (!uri_path)
        {
            uri_path = "/";
        }
        xlog_dbg("uri_path: %s", uri_path);

        if (strstr(uri_path, ".."))
        {
            xlog_dbg("path contains \"..\", ignored");
            http_error_code = HTTP_NOTFOUND;
            break;
        }

        if (!sopt->docroot)
        {
            xlog_err("root is null");
            http_error_code = HTTP_NOTFOUND;
            break;
        }

        evbuf = evbuffer_new();
        if (!evbuf)
        {
            xlog_err("evbuffer_new");
            break;
        }

        if (generate_html_of_path(evbuf, sopt->docroot, uri_path, &http_error_code) < 0)
        {
            xlog_err("generate html failed");
            http_error_code = HTTP_NOTFOUND;
            break;
        }

        std::string whole_path = std::string() + sopt->docroot + "/" + uri_path;
        struct stat st = {};
        if (stat(whole_path.c_str(), &st) < 0)
        {
            xlog_err("stat failed");
            http_error_code = HTTP_NOTFOUND;
            break;
        }

        if (S_ISDIR(st.st_mode))
        {
            evhttp_add_header(evhttp_request_get_output_headers(req),
                "Content-Type", "text/html");
        }
        else 
        {
            const char *type = guess_content_type(whole_path.c_str());
            evhttp_add_header(evhttp_request_get_output_headers(req),
                "Content-Type", type);
        }

        error = false;
    }
    while (0);

    if (error)
    {
        if (http_error_code != 0)
        {
            evhttp_send_error(req, http_error_code, nullptr);
        }
    }
    else 
    {
        evhttp_send_reply(req, HTTP_OK, "OK", evbuf);
    }

    if (decoded_uri_path)
    {
        free(decoded_uri_path);
        decoded_uri_path = nullptr;
    }

    if (parsesd_uri)
    {
        evhttp_uri_free(parsesd_uri);
        parsesd_uri = nullptr;
    }

    if (evbuf)
    {
        evbuffer_free(evbuf);
        evbuf = nullptr;
    }

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

        evhttp_set_cb(http, "/dump", dump_request_cb, nullptr);
        evhttp_set_gencb(http, send_document_cb, &sopt);

        handle_v4 = evhttp_bind_socket_with_handle(http, "0.0.0.0", sopt.port);
        handle_v6 = evhttp_bind_socket_with_handle(http, "::", sopt.port);

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

