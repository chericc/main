#pragma once

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class MainAlgManager {
   public:
    using FuncName = std::string;
    using Func = std::function<void()>;
    using Funcs = std::map<FuncName, Func>;
    static MainAlgManager& getInstance();
    void add(std::string const& module, Funcs funcs);

    void listDemos();
    void runAllDemo();

    /// Run specific modules by name (e.g. "heap", "longest_common_subsequence").
    /// Unknown modules are skipped with a warning.
    void runModules(std::vector<std::string> const& modules);

    /// Run a single module by name.
    /// Returns true if the module was found and executed.
    bool runModule(std::string const& module);

    /// Check if a module is registered.
    bool hasModule(std::string const& module) const;

   protected:
    static std::shared_ptr<MainAlgManager> instance_;

   private:
    using AlgDemos = std::map<std::string, Funcs>;
    AlgDemos alg_demos_;
};

/**
 * Helper for automatic algorithm registration via static initialization.
 *
 * Usage: put this at file scope in any .cpp file:
 *
 *   static StaticRegistrant _reg_xxx(SomeAlgo::registerTest);
 *
 * The registration function will be called automatically before main().
 */
class StaticRegistrant {
   public:
    template <typename Func>
    StaticRegistrant(Func&& func) {
        func();
    }
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