#include <SDL.h>

#include "xdisplay.hpp"
#include "xlog.hpp"

int main(int argc, char *argv[])
{
    std::vector<FILE*> fps = { stdout };

    FILE* fp = fopen("log.txt", "w");
    if (fp)
    {
        fps.push_back(fp);
    }

    xlog_setoutput(fps);

    XDisplay dis;

    std::string path = std::string() + RES_VIDEO_PATH + "/demo.mp4";

    dis.open(path);

    return dis.exec();
}
