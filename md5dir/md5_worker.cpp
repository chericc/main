#include "md5_worker.hpp"

#include <stdio.h>
#include <string.h>

#include <iostream>

#include "md5sum.hpp"
#include "xlog.h"

Md5Worker::~Md5Worker() { finish(); }

int Md5Worker::addPaths(std::list<FileItem>&& paths) {
    std::lock_guard<std::mutex> lock_call(mutex_call__);
    std::lock_guard<std::mutex> lock_items(mutex_items__);

    items__ = paths;

    return 0;
}

int Md5Worker::start(std::size_t jobs) {
    std::lock_guard<std::mutex> lock_call(mutex_call__);

    if (!threads__.empty()) {
        xlog_err("tasks not empty\n");
        return -1;
    }

    auto func = [this](int id) { this->workerThd(id); };

    for (std::size_t i = 0; i < jobs; ++i) {
        std::shared_ptr<std::thread> ptr_thread =
            std::make_shared<std::thread>(func, i);
        threads__.push_back(ptr_thread);
    }

    return 0;
}

int Md5Worker::finish() {
    std::lock_guard<std::mutex> lock_call(mutex_call__);

    for (auto it = threads__.begin(); it != threads__.end();) {
        if ((*it)->joinable()) {
            (*it)->join();
            it = threads__.erase(it);

            continue;
        } else {
            xlog_err("not joinable\n");
            return -1;
        }

        ++it;
    }

    return 0;
}

int Md5Worker::getResult(std::shared_ptr<std::list<Md5Result>>& result) {
    std::lock_guard<std::mutex> lock_call(mutex_call__);

    std::lock_guard<std::mutex> lock_result(mutex_results__);

    result = results__;
    results__.reset();

    return 0;
}

std::shared_ptr<FileItem> Md5Worker::getJob() {
    std::lock_guard<std::mutex> lock_item(mutex_items__);

    std::shared_ptr<FileItem> item_ptr;

    if (!items__.empty()) {
        item_ptr = std::make_shared<FileItem>(items__.front());
        items__.pop_front();
    }

    return item_ptr;
}

int Md5Worker::addResult(Md5Result&& result) {
    std::lock_guard<std::mutex> lock_result(mutex_results__);

    if (!results__) {
        results__ = std::make_shared<std::list<Md5Result>>();
    }

    results__->push_back(result);

    return 0;
}

int Md5Worker::workerThd(int id) {
    bool error_flag = 0;

    FILE* fp = nullptr;
    std::shared_ptr<FileItem> item_ptr;

    while (true) {
        Md5Result md5_result;

        if (nullptr != fp) {
            fclose(fp);
            fp = nullptr;
        }

        item_ptr = getJob();

        if (!item_ptr) {
            xlog_dbg("no task, thread [{}] fin\n", id);
            break;
        }

        if (nullptr == fp) {
            fp = fopen(item_ptr->file_path.c_str(), "r");
            if (nullptr == fp) {
                xlog_err("open file <{}> failed\n",
                         item_ptr->file_path.c_str());
                error_flag = true;
                continue;
            }
        }

        md5_result.worker_id = id;
        md5_result.file_path = item_ptr->file_path;
        md5_result.md5.fill(0);

        if (md5_stream(fp, md5_result.md5.data())) {
            xlog_err("md5 failed\n");
            break;
        }

#if 1
        do {
            char str[4 * MD5_LEN]{};
            for (std::size_t i = 0; i < md5_result.md5.size(); ++i) {
                snprintf(str + i * 2, 3, "%02hhx", md5_result.md5[i]);
            }

            printf("[%d]add result: %s %s\n", id, str,
                   md5_result.file_path.c_str());
        } while (false);
#endif

        addResult(std::move(md5_result));
    }

    if (nullptr != fp) {
        fclose(fp);
        fp = nullptr;
    }

    return error_flag ? -1 : 0;
}