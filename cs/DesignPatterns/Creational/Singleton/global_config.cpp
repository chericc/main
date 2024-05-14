#include "global_config.hpp"

// ??????
// compile error when using std::shared_ptr
// why?

// std::shared_ptr<GlobalConfig> GlobalConfig::config_;
GlobalConfig *GlobalConfig::config_ = nullptr;
std::mutex GlobalConfig::mutex_config_;

GlobalConfig& GlobalConfig::getInstance()
{
    if (!config_) 
    {
        std::lock_guard<std::mutex> lock(mutex_config_);
        if (!config_) 
        {
            // config_ = std::make_shared<GlobalConfig>();
            config_ = new GlobalConfig();
        }
    }
    return *config_;
}

std::string GlobalConfig::valueOf(std::string const& name)
{
    if (name == "name")
    {
        return "John";
    }
    else if (name == "sex")
    {
        return "Man";
    }
    else 
    {
        return "";
    }

    return "";
}