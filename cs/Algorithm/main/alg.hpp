#pragma once

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

class MainAlgManager {
   public:
    using FuncName = std::string;
    using Func = std::function<void()>;
    using Funcs = std::map<FuncName, Func>;
    static MainAlgManager& getInstance();
    void add(std::string const& module, Funcs funcs);

    void listDemos();
    void runAllDemo();

   protected:
    static std::shared_ptr<MainAlgManager> instance_;

   private:
    using AlgDemos = std::map<std::string, Funcs>;
    AlgDemos alg_demos_;
};

template <typename T_>
std::string output_elements(T_ const& c) {
    std::stringstream ss;
    for (auto const& ref : c) {
        ss << ref << " ";
    }
    return ss.str();
}

template <typename PtrType_, typename SizeType_>
std::string output_elements(PtrType_ data, SizeType_ begin, SizeType_ end) {
    std::stringstream ss;
    for (SizeType_ i = begin; i < end; ++i) {
        ss << data[i] << " ";
    }
    return ss.str();
}