#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

int64_t get_dir_size_recursively(const char *folder_path)
{
    int64_t total_size = 0;
    DIR *dir = NULL;
    int error_flag = 0;

    do {
        dir = opendir(folder_path);
        if (!dir) {
            printf("opendir failed\n");
            error_flag = 1;
            break;
        }

        struct dirent *dir_entry = NULL;
        while ((dir_entry = readdir(dir)) != NULL) {
            if ((0 == strcmp(dir_entry->d_name, ".")) || (0 == strcmp(dir_entry->d_name, ".."))) {
                continue;
            }

            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, dir_entry->d_name);

            struct stat sstat = {};
            int ret = stat(full_path, &sstat);
            if (ret != 0) {
                printf("stat path failed(%s)\n", full_path);
                error_flag = 1;
                break;
            }

            if (S_ISDIR(sstat.st_mode)) {
                int64_t subdir_size = get_dir_size_recursively(full_path);
                if (subdir_size < 0) {
                    printf("get sub dir size failed(%s)\n", full_path);
                    error_flag = 1;
                    break;
                }
                total_size += subdir_size;
            } else if (S_ISREG(sstat.st_mode)) {
                total_size += sstat.st_size;
            }
        }
    } while (0);

    if (dir) {
        closedir(dir);
        dir = NULL;
    }

    return error_flag ? -1 : total_size;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s [path of folder]\n", argv[0]);
        return -1;
    }

    uint64_t dir_size = get_dir_size_recursively(argv[1]);
    printf("dir_size: %" PRIu64 "\n", dir_size);

    return 0;
}
