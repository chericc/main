#include <xlog.h>

#include "global_config.hpp"

int main() {
    xlog_dbg("name: %s, sex: %s",
             GlobalConfig::getInstance().valueOf("name").c_str(),
             GlobalConfig::getInstance().valueOf("sex").c_str());
    return 0;
}