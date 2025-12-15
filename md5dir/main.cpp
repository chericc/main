#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <string>
#include <thread>

#include "md5_worker.hpp"
#include "search_dir.hpp"
#include "xlog.h"

int test(const char* path, int jobs) {
    int size = 0;

    std::string dir_path(path);
    std::list<FileItem> file_items;
    std::shared_ptr<std::list<Md5Result>> result;

    Md5Worker md5_worker;

    char str_now[32]{};
    char output_file[64]{};

    FILE* fp_output = nullptr;

    time_t tnow = time(NULL);
    struct tm tm_now {};
    localtime_r(&tnow, &tm_now);
    strftime(str_now, sizeof(str_now), "%F_%T", &tm_now);

    if (dir_path.size() >= 2 && (*--dir_path.end() == '/')) {
        dir_path = std::string(dir_path.c_str(), dir_path.length() - 1);
    }

    if (nullptr == fp_output) {
        snprintf(output_file, sizeof(output_file), "md5.log");
        fp_output = fopen(output_file, "a");
        if (nullptr == fp_output) {
            xlog_err("open failed: {}\n", output_file);
            return -1;
        }
    }

    search_dir(dir_path, file_items);

    size = (int)file_items.size();

    xlog_inf("%u items found\n", size);

    md5_worker.addPaths(std::move(file_items));
    md5_worker.start((std::size_t)jobs);
    md5_worker.finish();

    md5_worker.getResult(result);
    if (result) {
        result->sort();
    }

#if 1
    fprintf(fp_output, "%s\n", str_now);
    for (auto it = result->cbegin(); it != result->cend(); ++it) {
        char str[4 * MD5_LEN]{};
        for (std::size_t i = 0; i < it->md5.size(); ++i) {
            snprintf(str + i * 2, 3, "%02hhx", it->md5[i]);
        }
        fprintf(fp_output, "%s %s\n", str, it->file_path.c_str());
    }
    fprintf(fp_output, "\n");
#endif

    if (nullptr != fp_output) {
        fclose(fp_output);
        fp_output = nullptr;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    const char* dir_path = nullptr;
    int jobs = 1;

    if (argc != 3) {
        printf("Usage: %s <dir> <job>\n", argv[0]);
        return -1;
    }

    dir_path = argv[1];
    jobs = atoi(argv[2]);

    xlog_inf("searching paths: %s\n", dir_path);

    test(dir_path, jobs);

    return 0;
}