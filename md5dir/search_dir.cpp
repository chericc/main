#include "search_dir.hpp"

#include "platform.hpp"

#ifdef USE_LINUX_SYSCALLS__
#include <dirent.h>
#include <sys/types.h>
#endif

#include <limits.h>
#include <stdlib.h>

#include "xlog.h"

#ifdef USE_LINUX_SYSCALLS__

static int search_dir_recusive(const std::string& dir_path,
                               std::list<FileItem>& file_items) {
    DIR* dir = nullptr;
    struct dirent* dir_entry = nullptr;

    dir = opendir(dir_path.c_str());
    if (nullptr == dir) {
        xlog_err("open dir failed: <{}>\n", dir_path.c_str());
        return -1;
    }

    while ((dir_entry = readdir(dir)) != nullptr) {
        std::string entry_name = std::string(dir_entry->d_name);
        std::string entry_path;

        if ("." == entry_name) {
            continue;
        }
        if (".." == entry_name) {
            continue;
        }

        entry_path = dir_path + "/" + entry_name;

        if (DT_DIR == dir_entry->d_type) {
            // xdebug("-> %s\n", entry_path.c_str());
            search_dir_recusive(entry_path, file_items);
        } else if (DT_REG == dir_entry->d_type) {
            // xdebug ("-- %s\n", entry_path.c_str());
            FileItem item;
            item.file_path = std::string(entry_path);
            file_items.push_back(item);
        }

        continue;
    }

    if (nullptr != dir) {
        closedir(dir);
        dir = nullptr;
    }

    // xdebug ("<- %s\n", dir_path.c_str());

    return 0;
}

int search_dir(const std::string& dir_path, std::list<FileItem>& file_items) {
    std::string dir_path_real = std::string(dir_path.c_str());

    int ret = search_dir_recusive(dir_path_real, file_items);

    return ret;
}

#endif