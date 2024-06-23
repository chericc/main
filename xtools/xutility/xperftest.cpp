#include "xperftest.hpp"

#include <iomanip>
#include <sstream>

#include "xlog.hpp"

XPeriodPrint::XPeriodPrint(const char* file, int line, const char* function,
                           XLOG_LEVEL severity, const char* description) {
    tp_ = Clock::now();
    file_.assign(file);
    line_ = line;
    function_.assign(function);
    level_ = severity;
    desc_.assign(description);
    return;
}

XPeriodPrint::~XPeriodPrint() {
    Timepoint tp = Clock::now();
    // auto dur = tp - tp_;
    auto us =
        std::chrono::duration_cast<std::chrono::microseconds>(tp - tp_).count();
    auto sec =
        std::chrono::duration_cast<std::chrono::seconds>(tp - tp_).count();

    std::stringstream ss;
    ss << sec << "." << std::setw(6) << std::setfill('0') << us % 1000000
       << "s";

    xlog(level_, file_.c_str(), line_, function_.c_str(), "Period %s %s",
         ss.str().c_str(), desc_.c_str());

    return;
}