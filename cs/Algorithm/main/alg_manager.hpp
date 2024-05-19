#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>

class MainAlgManager
{
public:
    using FuncName = std::string;
    using Func = std::function<void()>;
    using Funcs = std::map<FuncName, Func>;
    static MainAlgManager& getInstance();
    void add(std::string const& module, Funcs funcs);

    void listDemos();
    void runDemo();
protected:
    static std::shared_ptr<MainAlgManager> instance_;
private:
    using AlgDemos = std::map<std::string, Funcs>;
    AlgDemos alg_demos_;
};

