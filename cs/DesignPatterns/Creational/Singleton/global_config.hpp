#pragma once

#include <memory>
#include <mutex>
#include <string>

class GlobalConfig {
   public:
    static GlobalConfig& getInstance();

    std::string valueOf(std::string const& name);

   protected:
    GlobalConfig() = default;

    static std::shared_ptr<GlobalConfig> config_;
    static std::mutex mutex_config_;
};