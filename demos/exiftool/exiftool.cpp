#include "exiftool.hpp"

#include <array>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <climits>

#include <unistd.h>
#include <csignal>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "xlog.h"
#include "xos_independent.hpp"
#include "json.hpp"

#define OK_OR_BREAK(call) \
    if (true) { \
        int ret = ((call)); \
        if (ret < 0) { \
            xlog_err(#call "failed: {}", ret); \
            break; \
        } \
    }

struct ExifTool::Ctx {
    static std::once_flag once_flag;

    int fdTo = -1;
    int fdFrom = -1;
    int fdFromErr = -1;

    int childPID = -1;
    int childWatchdog = -1;
};

std::once_flag ExifTool::Ctx::once_flag = {};

ExifTool::ExifTool()
{
    _ctx = std::make_shared<Ctx>();

    if (!init()) {
        xlog_err("init failed");
    }
}

ExifTool::~ExifTool()
{
    do {
        if (_ctx == nullptr) {
            break;
        }

        if (_ctx->childPID > 0) {
            kill(_ctx->childPID, SIGINT);
            xlog_dbg("wait child");
            waitpid(_ctx->childPID, nullptr, 0);
        }

        if (_ctx->childWatchdog > 0) {
            kill(_ctx->childWatchdog, SIGINT);
            xlog_dbg("wait watchdog");
            waitpid(_ctx->childWatchdog, nullptr, 0);
        }

        if (_ctx->fdTo >= 0) {
            close(_ctx->fdTo);
            _ctx->fdTo = -1;
        }
        if (_ctx->fdFrom >= 0) {
            close(_ctx->fdFrom);
            _ctx->fdFrom = -1;
        }
        if (_ctx->fdFromErr >= 0) {
            close(_ctx->fdFromErr);
            _ctx->fdFromErr = -1;
        }
    } while (false);
}

bool ExifTool::init()
{
    bool suc_flag = false;

    x_setthreadname("exifmain");

    do {
        std::array<int, 2> pipe_to = {};
        std::array<int, 2> pipe_from = {};
        std::array<int, 2> pipe_fromerr = {};

        OK_OR_BREAK(pipe(pipe_to.data()));
        OK_OR_BREAK(pipe(pipe_from.data()));
        OK_OR_BREAK(pipe(pipe_fromerr.data()));

        std::call_once(Ctx::once_flag, []() {
            signal(SIGPIPE, nullptr);
        });

        int childPID = fork();
        if (childPID < 0) {
            xlog_err("fork failed");
            break;
        }

        if (childPID == 0) {
            x_setthreadname("exifchild");

            // child process
            close(pipe_to.at(1)); // not write
            close(pipe_from.at(0)); // not read
            close(pipe_fromerr.at(0));
            dup2(pipe_to.at(0), STDIN_FILENO);
            dup2(pipe_from.at(1), STDOUT_FILENO);
            dup2(pipe_fromerr.at(1), STDERR_FILENO);
            close(pipe_to.at(0));
            close(pipe_from.at(1));
            close(pipe_fromerr.at(1));
            std::array<const char *, 7> args = {
                "exiftool",
                "-stay_open",
                "true",
                "-@",
                "-"
            };
            execvp(args.at(0), (char* const*)args.data());
            exit(0);
        } else {
            // this process
            close(pipe_to.at(0)); // not read
            close(pipe_from.at(1)); // not write
            close(pipe_fromerr.at(1));

            _ctx->fdTo = pipe_to.at(1);
            _ctx->fdFrom = pipe_from.at(0);
            _ctx->fdFromErr = pipe_fromerr.at(0);
            _ctx->childPID = childPID;

            int flags = fcntl(_ctx->fdFrom, F_GETFL, 0);
            fcntl(_ctx->fdFrom, F_SETFL, flags | O_NONBLOCK);

            // 如果进程异常退出，将exiftool进程也杀掉
            int pidThis = getpid();
            _ctx->childWatchdog = fork();
            if (0 == _ctx->childWatchdog) {
                x_setthreadname("exifwatchdog");
                while (true) {
                    sleep(1);
                    if (getppid() == pidThis) continue;
                    kill(_ctx->childPID, SIGINT);
                    exit(0);
                }
            }
        }

        suc_flag = true;
    } while (false);

    return suc_flag;
}

bool ExifTool::parse(const char *filename, int timeoutMs)
{
    bool suc_flag = false;
    do {
        // into realpath
        std::array<char, PATH_MAX + 1> path = {};
        realpath(filename, path.data());
        filename = path.data();

        if (!sendCommand(filename)) {
            xlog_err("send command failed");
            break;
        }

        auto startTime = std::chrono::steady_clock::now();

        std::string output;
        while (true) {
            do {
                if (!readOutput(output, timeoutMs)) {
                    break;
                }

                xlog_dbg("output: size={}, {}", 
                    (int)output.size(),
                    output.c_str());

                // parse json result
                auto json = nlohmann::json::parse(output, nullptr, false);
                if (json.is_discarded()) {
                    xlog_err("parse failed");
                    break;
                }

                if (json.empty()) {
                    xlog_err("json empty");
                    break;
                }

                xlog_dbg("json: size={}", (int)json.size());
            } while (false);

            auto passMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();
            xlog_dbg("pass: {} ms", (int)passMs);

            if (passMs > timeoutMs) {
                xlog_err("timeout");
                break;
            }
        }

        suc_flag = true;
    } while (false);

    xlog_dbg("parse end");

    return suc_flag;
}

bool ExifTool::sendCommand(const char *filename)
{
    do {
        std::vector<char> buf;
        buf.resize(1024);

        const char *cmds = "-json\n-l\n-D\n-execute\n";
        snprintf(buf.data(), buf.size(), "%s\n%s", filename, cmds);

        write(_ctx->fdTo, buf.data(), buf.size());
    } while (false);

    return true;
}

bool ExifTool::readOutput(std::string &result, int timeoutMs)
{
    bool error_flag = false;
    std::string output;
    do {
        std::vector<char> buf;
        buf.resize(1024);
        bool got_result = false;

        const char *needle = "{ready}";

        auto startTime = std::chrono::steady_clock::now();

        while (true) {
            xlog_dbg("read");

            do {
                errno = 0;
                ssize_t nret = read(_ctx->fdFrom, buf.data(), buf.size());
                if (nret > 0) {
                    output.append(buf.data());
                    size_t pos = output.find(needle);
                    if (pos != std::string::npos) {
                        output.resize(pos);
                        got_result = true;
                    }
                    break;
                }

                if (errno == EAGAIN
                    || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(READ_WAIT_MS));
                    break;
                }

                xlog_err("error in read: ret={}, {}", (int)nret, strerror(errno));
                error_flag = true;
                break;
            } while (false);

            auto timePass = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();
            if (timePass > timeoutMs) {
                break;
            }

            if (error_flag || got_result) {
                break;
            }
        }
    } while (false);

    result.swap(output);

    return !error_flag;
}