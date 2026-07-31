#include <string>
#include <vector>

#include "alg.hpp"
#include "xlog.hpp"

int main(int argc, char* argv[]) {
    xlog_dbg("main start");

    // All algorithm modules register themselves via static initialization
    // before main() is called. Nothing to do here manually.

    // Collect command-line arguments (skip program name)
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    auto& mgr = MainAlgManager::getInstance();

    // Check if --help or --list appears anywhere in args
    bool list_only = false;
    std::vector<std::string> modules;
    for (auto const& a : args) {
        if (a == "--list") {
            list_only = true;
        } else if (a == "--help" || a == "-h") {
            xlog_dbg("Usage: {} [module_name ...] [--list]", argv[0]);
            xlog_dbg("  no args : run all registered algorithm demos");
            xlog_dbg("  --list  : list available modules (don't run)");
            xlog_dbg("  mod1..  : run only the specified module(s)");
            return 0;
        } else {
            modules.push_back(a);
        }
    }

    if (list_only) {
        // --list: only display available modules, don't run anything
        mgr.listDemos();
        return 0;
    }

    if (modules.empty()) {
        // No module names: run everything
        xlog_dbg("running all demos...");
        mgr.runAllDemo();
    } else {
        // Specific module names provided: run only those
        xlog_dbg("running selected demos...");
        mgr.runModules(modules);
    }

    return 0;
}