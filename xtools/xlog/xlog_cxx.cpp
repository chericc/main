#include "xlog_cxx.hpp"

#include <streambuf>

static const std::size_t kMaxLogMessageLen = 30000;

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
        : std::ostream(nullptr), streambuf_(buf, len)/*, ctr_(ctr), self_(this)*/ {
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
    // int64_t ctr_;      // Counter hack (for the LOG_EVERY_X() macro)
    // LogStream* self_;  // Consistency check hack
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

    data_->basename_ = xlog_basename(file);
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