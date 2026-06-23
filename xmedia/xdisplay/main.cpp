#include "xdisplay.hpp"
#include "xlog.h"

#if defined(X_WINDOWS_PLATFORM)
#pragma comment(linker, "/subsystem:console")
#endif

int main(int argc, char* argv[]) {
    XDisplay dis;

    std::string path = std::string() + RES_VIDEO_PATH + "/demo.mp4";

    if (dis.open(path) < 0) {
        xlog_err("open failed");
        return -1;
    }

    return dis.exec();
    ;
}
