#include "global_config.hpp"

// ??????
// compile error when using std::shared_ptr
// why?
// 这是一个C++11标准存在的问题，由于make_shared并不是类GlobalConfig的友元函数，因此无法调用
// 其私有的构造函数
// 解决办法
// 1. 直接调用new对shared_ptr进行构造
// 2.
// 构造一个struct子类，将构造函数转换为公共的（利用了子类可以访问protected的特性）
// 这里我们使用第二种方法

std::shared_ptr<GlobalConfig> GlobalConfig::config_;
// GlobalConfig *GlobalConfig::config_ = nullptr;
std::mutex GlobalConfig::mutex_config_;

namespace {
struct EnableShared : public GlobalConfig {};
}  // namespace

GlobalConfig& GlobalConfig::getInstance() {
    if (!config_) {
        std::lock_guard<std::mutex> lock(mutex_config_);
        if (!config_) {
            config_ = std::make_shared<EnableShared>();
        }
    }
    return *config_;
}

std::string GlobalConfig::valueOf(std::string const& name) {
    if (name == "name") {
        return "John";
    } else if (name == "sex") {
        return "Man";
    } else {
        return "";
    }

    return "";
}