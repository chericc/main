#include <libgen.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <climits>
#include <array>

bool mkdir_p(const char *path, int mode) 
{   
    bool okFlag = false;
    do {
        std::array<char, PATH_MAX> buf = {};
        size_t len;

        len = snprintf(buf.data(), buf.size(), "%s", path);
        if (len <= 0) {
            break;
        }

        if (buf[len - 1] == '/') {
            buf[len - 1] = 0;
        }

        bool errorFlag = false;
        for (char *pit = buf.data() + 1; *pit != 0; pit++) {
            if (*pit == '/') {
                *pit = 0;
                if (mkdir(buf.data(), mode) != 0 && errno != EEXIST) {
                    errorFlag = true;
                    break;
                }
                *pit = '/';
            }
        }

        if (errorFlag) {
            break;
        }

        if (mkdir(buf.data(), mode) != 0 && errno != EEXIST) {
            break;
        }

        okFlag = true;
    } while (false);

    return okFlag;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("%s [path]\n", argv[0]);
        return 0;
    }

    mkdir_p(argv[1], 0777);
    return 0;
}