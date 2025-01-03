#include <libgen.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int mkdir_p(const char *path, int mode) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    // Copy path and ensure it is mutable
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (len <= 0) {
        return -1;
    }

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    // Create each directory level
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }

    // Create the final directory
    if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
        return -1;
    }

    return 0;
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