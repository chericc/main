#include "xlog.hpp"

// #define __STDC_WANT_LIB_EXT2__ 1

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <cstring>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <streambuf>
#include <thread>
#include <vector>

static const std::size_t kMaxLogMessageLen = 30000;
static unsigned int s_log_mask = 0xffffffff;
static std::vector<FILE*> s_fps = {stdout};
static std::mutex s_call_mutex;

static const char* const_basename(const char* filepath) {
    const char* base = strrchr(filepath, '/');
    if (!base) base = strrchr(filepath, '\\');
    return base ? (base + 1) : filepath;
}

static const char* xlog_getlevel(XLOG_LEVEL level) {
    int i = 0;

    /* Make sure this two arrays has the same number of items. */

    const char* array_name[] = {"trc", "dbg", "log", "inf",
                                "war", "err", "cri"};
    const XLOG_LEVEL array_level[] = {
        XLOG_LEVEL_TRACE,       XLOG_LEVEL_DEBUG,   XLOG_LEVEL_LOG,
        XLOG_LEVEL_INFORMATION, XLOG_LEVEL_WARNING, XLOG_LEVEL_ERROR,
        XLOG_LEVEL_CRITICAL,
    };

    static_assert(sizeof(array_name) / sizeof(array_name[0]) ==
                      sizeof(array_level) / sizeof(array_level[0]),
                  "size error");

    for (i = 0; i < XLOG_LEVEL_BUTT; ++i) {
        if (array_level[i] == level) {
            return array_name[i];
        }
    }

    return "unknown";
}

static std::string now_str() {
    auto now = std::chrono::system_clock::now();
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(now)
                    .time_since_epoch()
                    .count();
    auto msecs = std::chrono::time_point_cast<std::chrono::milliseconds>(now)
                     .time_since_epoch()
                     .count() %
                 1000;

    std::stringstream ss;
    ss << secs << ":" << std::setw(3) << std::setfill('0') << msecs;
    return ss.str();
}

static std::string trd_str() {
    std::stringstream ss;
    std::thread::id tid = std::this_thread::get_id();
    ss << tid;
    return ss.str();
}

void xlog_setmask(unsigned int mask) {
    std::unique_lock<std::mutex> lock(s_call_mutex);
    s_log_mask = mask;
    return;
}

unsigned int xlog_getmask() {
    std::unique_lock<std::mutex> lock(s_call_mutex);
    return s_log_mask;
}

void xlog_setoutput(const std::vector<FILE*>& fps) {
    std::unique_lock<std::mutex> lock(s_call_mutex);

    // close all output
    for (auto& it : s_fps) {
        if (it != stdin && it != stdout && it != stderr) {
            fclose(it);
        }
    }
    s_fps.clear();
    s_fps.shrink_to_fit();

    for (auto const& it : fps) {
        if (!it) {
            continue;
        }
        s_fps.push_back(it);
    }

    return;
}

void xlog(XLOG_LEVEL level, const char* file, int line, const char* func,
          const char* format, ...) {
    va_list ap;

    if (!(level & s_log_mask)) {
        return;
    }

    std::unique_lock<std::mutex> lock(s_call_mutex);

    for (auto& it : s_fps) {
        fprintf(it, "[%s]", now_str().c_str());
        fprintf(it, "[%s]", trd_str().c_str());
        fprintf(it, "[%s]", xlog_getlevel(level));
        fprintf(it, "[%s %d %s] ", const_basename(file), line, func);
        va_start(ap, format);
        vfprintf(it, format, ap);
        va_end(ap);
        fprintf(it, "\n");
        fflush(it);
    }

    return;
}

class LogStreamBuf : public std::streambuf {
   public:
    // REQUIREMENTS: "len" must be >= 2 to account for the '\n' and '\0'.
    // LogStreamBuf(char* buf, int len) { setp(buf, buf + len - 2); }

    // REQUIREMENTS: "len" must be >= 2 to account for the '\0'.
    LogStreamBuf(char* buf, int len) { setp(buf, buf + len - 2); }

    // This effectively ignores overflow.
    int_type overflow(int_type ch) { return ch; }

    // Legacy public ostrstream method.
    size_t pcount() const { return pptr() - pbase(); }
    char* pbase() const { return std::streambuf::pbase(); }
};

class LogStream : public std::ostream {
   public:
    LogStream(char* buf, int len, int64_t ctr)
        : std::ostream(nullptr), streambuf_(buf, len), ctr_(ctr), self_(this) {
        rdbuf(&streambuf_);
    }

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    // Legacy std::streambuf methods.
    size_t pcount() const { return streambuf_.pcount(); }
    char* pbase() const { return streambuf_.pbase(); }
    char* str() const { return pbase(); }

   private:
    LogStreamBuf streambuf_;
    int64_t ctr_;      // Counter hack (for the LOG_EVERY_X() macro)
    LogStream* self_;  // Consistency check hack
};

struct XLogMessageData {
    XLogMessageData() : stream_(message_text_, kMaxLogMessageLen, 0) {}
    // Buffer space; contains complete message text.
    char message_text_[kMaxLogMessageLen + 1];
    LogStream stream_;
    XLOG_LEVEL severity_;  // What level is this XLogMessage logged at?
    int line_;             // line number where logging call is.

    size_t num_prefix_chars_;     // # of chars of prefix in this message
    size_t num_chars_to_log_;     // # of chars of msg to send to log
    size_t num_chars_to_syslog_;  // # of chars of msg to send to syslog
    const char* basename_;        // basename of file that called LOG
    const char* fullname_;        // fullname of file that called LOG
    const char* function_;

    XLogMessageData(const XLogMessageData&) = delete;
    XLogMessageData& operator=(const XLogMessageData&) = delete;
};

XLogMessage::XLogMessage(const char* file, int line, const char* function,
                         XLOG_LEVEL severity) {
    // Init(file, line, severity, &XLogMessage::SendToLog);
    data_ = new XLogMessageData;

    data_->basename_ = const_basename(file);
    data_->fullname_ = file;
    data_->line_ = line;
    data_->severity_ = severity;
    data_->function_ = function;
}

XLogMessage::~XLogMessage() {
    data_->num_chars_to_log_ = data_->stream_.pcount();
    data_->message_text_[data_->num_chars_to_log_] = '\0';
    // std::cout << "hahaha>>" << data_->basename_ << ":" << data_->line_ << " "
    // << data_->message_text_ << "\r\n";
    xlog(data_->severity_, data_->basename_, data_->line_, data_->function_,
         "%s", data_->message_text_);
    delete data_;
}

std::ostream& XLogMessage::stream() { return data_->stream_; }