#include <regex>

#include "rtsp_client_wrapper.h"
#include "xlog.h"
#include "packet_stable.hpp"

namespace {

struct RtspUrl {
    std::string protocol;
    std::string username;
    std::string password;
    std::string host;
    int port;
    std::string path;
};

RtspUrl parseRtspUrl(const std::string& url) {
    RtspUrl result = {"rtsp", "", "", "", 554, ""}; // 默认值

    // 正则表达式解释：
    // ^(rtsp|rtsps)://             协议
    // (?:([^:@]+)(?::([^@]+))?@)?  [用户名[:密码]@] (可选)
    // ([^:/]+)                     主机名/IP
    // (?::(\d+))?                  [:端口] (可选)
    // (.*)                         /路径 (可选)
    std::regex url_regex(R"(^(rtsp|rtsps)://(?:([^:@]+)(?::([^@]+))?@)?([^:/]+)(?::(\d+))?(.*)$)", std::regex::icase);
    std::smatch match;

    if (std::regex_match(url, match, url_regex)) {
        result.protocol = match[1];
        result.username = match[2];
        result.password = match[3];
        result.host = match[4];
        
        // 如果提供了端口则解析，否则保留默认 554
        if (match[5].matched) {
            result.port = std::stoi(match[5]);
        }
        
        result.path = match[6].matched ? match[6].str() : "/";
    }

    return result;
}


void data_cb(int ch, const char *encoding, const void *data, size_t bytes)
{
    // xlog_dbg("data: size={}", (int)bytes);

    if (1) {
        using Clock = std::chrono::steady_clock;
        auto now = Clock::now();
        static Clock::time_point tp_last = Clock::now();

        auto passed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - tp_last).count();
        xlog_dbg("passed={} ms(size={})", (int)passed_ms, (int)bytes);
        tp_last = now;
    }

    constexpr int ch_num = 4;
    static FILE *fp_array[ch_num];

    const char *encode_name = encoding;

    if (ch >= 0 && ch < ch_num) {
        FILE *&fp = fp_array[ch];
        if (!fp) {
            char filename[64];
            snprintf(filename, sizeof(filename), "./ch_%d.%s", ch, encode_name);
            fp = fopen(filename, "w");
        }
        if (fp) {
            fwrite(data, 1, bytes, fp);
            fflush(fp);
        }
    }

    return ;
}

void printUsage(int argc, char *argv[])
{
    xlog_dbg("usage: {} [url]\n"
        "url: rtsp://[username:password@]host[:port]/path");
}

}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printUsage(argc, argv);
        return 0;
    }

    auto url = parseRtspUrl(argv[1]);

    xlog_dbg("host: {}, path: {}, port: {}, user/pass: {}/{}", 
        url.host, url.path, url.port, url.username, url.password);

    rtsp_client_wrapper_param param = {};
    param.host = url.host.c_str();
    param.file = url.path.c_str();
    param.port = url.port;
    param.username = url.username.c_str();
    param.password = url.password.c_str();
    param.data_cb = data_cb;
    rtsp_client_wrapper_handle_t handle = rtsp_client_wrapper_start(&param);

    getchar();
    getchar();

    rtsp_client_wrapper_destroy(handle);

    return 0;
}