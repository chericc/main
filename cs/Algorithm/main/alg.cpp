#include "alg.hpp"

#include "xlog.hpp"

std::shared_ptr<MainAlgManager> MainAlgManager::instance_;

MainAlgManager& MainAlgManager::getInstance() {
    struct MakeShared : public MainAlgManager {};

    if (!instance_) {
        instance_ = std::make_shared<MakeShared>();
    }

    return *instance_;
}

void MainAlgManager::add(std::string const& module, Funcs funcs) {
    alg_demos_[module] = std::move(funcs);
}

void MainAlgManager::listDemos() {
    int module_index = 0;
    int func_index = 0;

    for (auto it = alg_demos_.begin(); it != alg_demos_.end();
         ++it, ++module_index) {
        xlog_dbg("module: %s(%d)", it->first.c_str(), module_index);
        for (auto func_it = it->second.begin(); func_it != it->second.end();
             ++func_it, ++func_index) {
            xlog_dbg("|--func: %s(%d)", func_it->first.c_str(), func_index);
        }
    }
}

void MainAlgManager::runAllDemo() {
    int module_index = 0;
    int func_index = 0;

    for (auto it = alg_demos_.begin(); it != alg_demos_.end();
         ++it, ++module_index) {
        xlog_dbg("module: %s(%d)", it->first.c_str(), module_index);
        for (auto func_it = it->second.begin(); func_it != it->second.end();
             ++func_it, ++func_index) {
            xlog_dbg("|--func: %s(%d)", func_it->first.c_str(), func_index);

            func_it->second();
        }
    }
}