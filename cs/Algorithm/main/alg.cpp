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
        xlog_dbg("module: {}({})", it->first.c_str(), module_index);
        for (auto func_it = it->second.begin(); func_it != it->second.end();
             ++func_it, ++func_index) {
            xlog_dbg("|--func: {}({})", func_it->first.c_str(), func_index);
        }
    }
}

void MainAlgManager::runAllDemo() {
    int module_index = 0;
    int func_index = 0;

    for (auto it = alg_demos_.begin(); it != alg_demos_.end();
         ++it, ++module_index) {
        xlog_dbg("module: {}({})", it->first.c_str(), module_index);
        for (auto func_it = it->second.begin(); func_it != it->second.end();
             ++func_it, ++func_index) {
            xlog_dbg("|--func: {}({})", func_it->first.c_str(), func_index);

            func_it->second();
        }
    }
}

void MainAlgManager::runModules(std::vector<std::string> const& modules) {
    for (auto const& mod : modules) {
        runModule(mod);
    }
}

bool MainAlgManager::runModule(std::string const& module) {
    auto it = alg_demos_.find(module);
    if (it == alg_demos_.end()) {
        xlog_warn("module '{}' not found, skip", module.c_str());
        return false;
    }

    xlog_dbg("module: {}", module.c_str());
    for (auto& [name, func] : it->second) {
        xlog_dbg("|--func: {}", name.c_str());
        func();
    }
    return true;
}

bool MainAlgManager::hasModule(std::string const& module) const {
    return alg_demos_.find(module) != alg_demos_.end();
}