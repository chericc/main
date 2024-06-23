#include "xdisplay.hpp"
#include "xlog.hpp"

#if defined(X_WINDOWS_PLATFORM)
#pragma comment(linker, "/subsystem:console")
#endif

int main(int argc, char* argv[]) {
    std::vector<FILE*> fps = {stdout};

    FILE* fp = fopen("log.txt", "w");
    if (fp) {
        fps.push_back(fp);
    }

    xlog_setoutput(fps);

    XDisplay dis;

    std::string path = std::string() + RES_VIDEO_PATH + "/demo.mp4";

    if (dis.open(path) < 0) {
        xlog_err("open failed");
        return -1;
    }

    return dis.exec();
    ;
}
