#pragma once

#include <string>
#include <memory>
#include <mutex>

class GlobalConfig
{
public:
    static GlobalConfig &getInstance();

    GlobalConfig& operator=(GlobalConfig&) = delete;

    std::string valueOf(std::string const& name);
private:
    GlobalConfig() = default;

    // static std::shared_ptr<GlobalConfig> config_;
    static GlobalConfig *config_;
    static std::mutex mutex_config_;
};