#pragma once

#include <string>
#include <memory>
#include <mutex>

class GlobalConfig
{
public:
    static GlobalConfig &getInstance();

    std::string valueOf(std::string const& name);
protected:
    GlobalConfig() = default;

    static std::shared_ptr<GlobalConfig> config_;
    static std::mutex mutex_config_;
};