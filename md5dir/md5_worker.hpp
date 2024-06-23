#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include "def.hpp"

class Md5Worker {
   public:
    ~Md5Worker();

    int addPaths(std::list<FileItem>&& paths);
    int start(std::size_t jobs = 1);
    int finish();
    int getResult(std::shared_ptr<std::list<Md5Result>>& result);

   private:
    std::shared_ptr<FileItem> getJob();
    int addResult(Md5Result&& result);
    int workerThd(int id);

   private:
    std::mutex mutex_call__;
    std::mutex mutex_items__;
    std::mutex mutex_results__;
    std::list<FileItem> items__;

    std::shared_ptr<std::list<Md5Result>> results__;
    std::list<std::shared_ptr<std::thread>> threads__;
};